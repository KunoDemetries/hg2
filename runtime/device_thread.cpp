// Host device threads for the coarse EE / device split (see device_link.hpp).
// Stage B applies VIF1 parsing and VU1 execution; stage C applies GIF packet
// assembly, GS work and host closures. Together they perform the synchronous
// path's VIF1/GIF/GS calls in exactly the original submission order.
#include "hg/device_thread.hpp"
#include "hg/gs_acceleration.hpp"
#include "hg/gs_triangle_acceleration.hpp"
#include "hg/vif.hpp"
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <stdexcept>
#include <thread>
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#include <immintrin.h>
#define HG_DEVICE_PAUSE() _mm_pause()
#else
#define HG_DEVICE_PAUSE() std::this_thread::yield()
#endif

namespace hg {
namespace {
struct DeviceCall {
    std::function<void()> work;
    bool sync=false;
    bool transport=false; // Completed by stage B itself (VIF1/GIF transport state only).
    std::exception_ptr error;
    std::atomic<bool> done{false};
};

// Consumer side of one single-producer/single-consumer command ring.
// HG-DIAG-091: host wait-time totals, printed at stop only with HG_DEVICE_STATS=1.
// Clocks are read only around waits; no guest-state or ordering effect.
inline std::uint64_t now_ns() {
    return std::uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
}
struct Consumer {
    std::atomic<std::uint64_t> idle_ns{0},sleeps{0},commands{0};
    std::mutex mutex;
    std::condition_variable wake;
    std::atomic<bool> sleeping{false},stopping{false},failed{false};
    std::exception_ptr error; // Written before failed is released.
    std::thread thread;
    void fail() {
        if(!failed.load(std::memory_order_relaxed)) {
            error=std::current_exception();failed.store(true,std::memory_order_release);
        }
    }
    bool has_failed() const {return failed.load(std::memory_order_relaxed);}
    void notify() {
        if(sleeping.load(std::memory_order_seq_cst)) {
            std::lock_guard<std::mutex> lock(mutex);wake.notify_one();
        }
    }
    // execute(at) applies the command at ring position `at` and returns how many
    // entries it consumed (a command plus any inline payload entries).
    template<class Execute,class Idle>
    void run(std::atomic<std::uint64_t>& write_index,std::atomic<std::uint64_t>& read_index,
             Execute&& execute,Idle&& idle) {
        std::uint64_t at=0,stored=0;
        for(;;) {
            auto end=write_index.load(std::memory_order_acquire);
            if(at==end) {
                read_index.store(at,std::memory_order_release);
                idle();
                const auto idle_start=now_ns();
                for(unsigned spin=0;spin<256 && at==end && !stopping.load(std::memory_order_relaxed);++spin) {
                    HG_DEVICE_PAUSE();end=write_index.load(std::memory_order_acquire);
                }
                if(at!=end){idle_ns+=now_ns()-idle_start;continue;}
                ++sleeps;
                sleeping.store(true,std::memory_order_seq_cst);
                if(write_index.load(std::memory_order_seq_cst)==at && !stopping.load(std::memory_order_seq_cst)) {
                    std::unique_lock<std::mutex> lock(mutex);
                    wake.wait(lock,[&] {
                        return write_index.load(std::memory_order_seq_cst)!=at||stopping.load(std::memory_order_seq_cst);
                    });
                }
                sleeping.store(false,std::memory_order_relaxed);
                idle_ns+=now_ns()-idle_start;
                if(stopping.load(std::memory_order_seq_cst) && write_index.load(std::memory_order_seq_cst)==at)break;
                continue;
            }
            commands+=end-at;
            while(at!=end) {
                at+=execute(at);
                if(at-stored>=256){read_index.store(at,std::memory_order_release);stored=at;}
            }
        }
        read_index.store(at,std::memory_order_release);
    }
    void stop() {
        stopping.store(true,std::memory_order_seq_cst);
        {std::lock_guard<std::mutex> lock(mutex);wake.notify_one();}
        if(thread.joinable())thread.join();
    }
};

class DeviceThread final : public DeviceLink, GifForward {
    Vif1Path& vif1;
    GifPath& gif; // Real path, owned by stage C.
    GsRegisterState& gs;
    GifPath proxy; // Stage B's boundary-tracking proxy.
    std::function<void(bool)> context;
    GsSpriteAccelerator* sprite=gs_sprite_accelerator;
    GsTriangleAccelerator* triangle=gs_triangle_accelerator;
    Consumer b,c;
    // Stage B -> C ring (B produces).
    std::vector<DeviceCommand> c_ring=std::vector<DeviceCommand>(capacity);
    alignas(64) std::atomic<std::uint64_t> c_write{0};
    alignas(64) std::atomic<std::uint64_t> c_read{0};
    alignas(64) std::uint64_t c_produced=0,c_read_cache=0,c_published=0;
    // Sync-call completion (stage C -> EE thread).
    std::mutex done_mutex;
    std::condition_variable done_wake;
    std::atomic<bool> ee_waiting{false};
    bool stopped=false;
    std::uint64_t join_wait_ns=0,space_wait_ns=0,c_space_wait_ns=0,space_waits=0,transport_reads=0;

    void c_publish() {
        if(c_produced==c_published)return;
        c_write.store(c_produced,std::memory_order_seq_cst);c_published=c_produced;c.notify();
    }
    void c_push(const DeviceCommand& command) {
        if(c_produced-c_read_cache>=capacity) {
            const auto start=now_ns();
            c_publish();
            while(c_produced-c_read.load(std::memory_order_acquire)>=capacity)std::this_thread::yield();
            c_read_cache=c_read.load(std::memory_order_acquire);
            c_space_wait_ns+=now_ns()-start;
        }
        c_ring[c_produced&mask]=command;++c_produced;
        if(c_produced-c_published>=512)c_publish();
    }
    void forward_qword(std::uint64_t low,std::uint64_t high) override {
        c_push({low,high,std::uint32_t(DeviceKind::gif_qword),0});
    }
    void forward_words(const std::uint32_t* words,std::size_t qwords) override {
        // One header plus inline payload entries, published only as a whole.
        const auto need=std::uint64_t(qwords)+1;
        if(need>capacity)throw std::runtime_error("host GIF block exceeds device ring capacity");
        if(c_produced+need-c_read_cache>capacity) {
            const auto start=now_ns();
            c_publish();
            while(c_produced+need-c_read.load(std::memory_order_acquire)>capacity)std::this_thread::yield();
            c_read_cache=c_read.load(std::memory_order_acquire);
            c_space_wait_ns+=now_ns()-start;
        }
        c_ring[c_produced&mask]={qwords,0,std::uint32_t(DeviceKind::gif_words),0};
        for(std::size_t n=0;n<qwords;++n,words+=4) {
            auto& entry=c_ring[(c_produced+1+n)&mask];
            entry.a=std::uint64_t(words[0])|(std::uint64_t(words[1])<<32);
            entry.b=std::uint64_t(words[2])|(std::uint64_t(words[3])<<32);
        }
        c_produced+=need;
        if(c_produced-c_published>=512)c_publish();
    }
    void complete(DeviceCall& call) {
        call.done.store(true,std::memory_order_seq_cst);
        if(ee_waiting.load(std::memory_order_seq_cst)) {
            std::lock_guard<std::mutex> lock(done_mutex);done_wake.notify_all();
        }
    }
    std::uint64_t execute_b(std::uint64_t at) {
        execute_b_command(ring[at&mask]);
        return 1;
    }
    void execute_b_command(const DeviceCommand& command) {
        const auto kind=DeviceKind(command.kind);
        if(kind==DeviceKind::call) {
            auto* call=reinterpret_cast<DeviceCall*>(command.a);
            if(call->transport) {
                try {call->work();} catch(...) {call->error=std::current_exception();}
                complete(*call);
            } else if(call->sync||!b.has_failed()) {c_push(command);if(call->sync)c_publish();}
            else delete call;
            return;
        }
        // After a failure, the synchronous path would have stopped; apply nothing more.
        if(b.has_failed())return;
        try {
            switch(kind) {
            case DeviceKind::vif_qword:vif1.submit_qword(command.a,command.b,proxy,gs);break;
            case DeviceKind::vif_tag:vif1.submit_dma_tag(command.a,proxy,gs);break;
            case DeviceKind::vif_register:vif1.write_register(command.x,std::uint32_t(command.a));break;
            case DeviceKind::gif_qword:proxy.submit_qword(command.a,command.b,gs);break;
            case DeviceKind::gif_control:proxy.write_control(std::uint32_t(command.a));c_push(command);break;
            case DeviceKind::gs_privileged:case DeviceKind::gs_csr_events:case DeviceKind::gs_imr:
            case DeviceKind::rasterize:c_push(command);break;
            default:throw std::runtime_error("invalid host device command");
            }
        } catch(...) {b.fail();}
    }
    std::uint64_t execute_c(std::uint64_t at) {
        const auto& command=c_ring[at&mask];
        const auto kind=DeviceKind(command.kind);
        if(kind==DeviceKind::call) {
            auto* call=reinterpret_cast<DeviceCall*>(command.a);
            if(call->sync) {
                // Diagnostics, joins and dumps also run after a failure.
                try {if(call->work)call->work();} catch(...) {call->error=std::current_exception();}
                complete(*call);
                return 1;
            }
            if(!c.has_failed()) {try {call->work();} catch(...) {c.fail();}}
            delete call;return 1;
        }
        if(kind==DeviceKind::gif_words) {
            const auto qwords=command.a;
            if(!c.has_failed()) {
                try {
                    for(std::uint64_t n=1;n<=qwords;++n) {
                        const auto& entry=c_ring[(at+n)&mask];
                        gif.submit_qword(entry.a,entry.b,gs);
                    }
                } catch(...) {c.fail();}
            }
            return qwords+1;
        }
        if(c.has_failed())return 1;
        try {
            switch(kind) {
            case DeviceKind::gif_qword:gif.submit_qword(command.a,command.b,gs);break;
            case DeviceKind::gif_control:gif.write_control(std::uint32_t(command.a));break;
            case DeviceKind::gs_privileged:gs.write_privileged(command.x,command.a);break;
            case DeviceKind::gs_csr_events:gs.privileged_csr_events|=command.x;break;
            case DeviceKind::gs_imr:gs.privileged_imr=command.a;break;
            case DeviceKind::rasterize:gs.rasterize_pending_draws();break;
            default:throw std::runtime_error("invalid host GS command");
            }
        } catch(...) {c.fail();}
        return 1;
    }
    // GS work precedes B's in the stream whenever both failed (B stops forwarding).
    void rethrow_failure() {
        if(c.failed.load(std::memory_order_acquire))std::rethrow_exception(c.error);
        if(b.failed.load(std::memory_order_acquire))std::rethrow_exception(b.error);
    }
    void publish_b() {
        write_index.store(produced,std::memory_order_seq_cst);published=produced;b.notify();
    }
    void wait_call(DeviceCall& call) {
        for(unsigned spin=0;spin<256;++spin) {
            if(call.done.load(std::memory_order_acquire))return;
            HG_DEVICE_PAUSE();
        }
        ee_waiting.store(true,std::memory_order_seq_cst);
        if(!call.done.load(std::memory_order_seq_cst)) {
            std::unique_lock<std::mutex> lock(done_mutex);
            done_wake.wait(lock,[&]{return call.done.load(std::memory_order_seq_cst);});
        }
        ee_waiting.store(false,std::memory_order_relaxed);
    }
    void sync_call(DeviceCall& call) {
        const bool due=raster_due;
        push(DeviceKind::call,0,std::uint64_t(reinterpret_cast<std::uintptr_t>(&call)));
        raster_due=due;
        const auto start=now_ns();
        publish_b();wait_call(call);
        join_wait_ns+=now_ns()-start;
    }
public:
    DeviceThread(Vif1Path& v,GifPath& g,GsRegisterState& s,std::function<void(bool)> context_switch)
        :vif1(v),gif(g),gs(s),context(std::move(context_switch)) {
        proxy.forward=this;
        // The real path may already hold a partial packet; mirror its boundary state.
        if(!gif.pending.empty())throw std::runtime_error("host device thread requires an idle GIF path");
        c.thread=std::thread([this] {
            if(context)context(true);
            gs_sprite_accelerator=sprite;gs_triangle_accelerator=triangle;
            c.run(c_write,c_read,[this](std::uint64_t at){return execute_c(at);},[]{});
            gs_triangle_accelerator=nullptr;gs_sprite_accelerator=nullptr;
            if(context)context(false);
        });
        b.thread=std::thread([this] {
            b.run(write_index,read_index,[this](std::uint64_t at){return execute_b(at);},
                  [this]{c_publish();});
            c_publish();
        });
    }
    ~DeviceThread() override {stop();}
    void publish_slow() override {
        publish_b();
        rethrow_failure();
    }
    void wait_space() override {
        const auto start=now_ns();
        publish_b();
        // Full ring: stage B is busy and publishes progress every 256 commands.
        while(produced-read_index.load(std::memory_order_acquire)>=capacity)std::this_thread::yield();
        read_cache=read_index.load(std::memory_order_acquire);
        space_wait_ns+=now_ns()-start;++space_waits;
    }
    bool stats=[]{const char* v=std::getenv("HG_DEVICE_STATS");return v&&v[0]=='1';}();
    std::uint64_t next_stats_ns=0;
    void print_stats(const char* label) {
        std::fprintf(stderr,"Host device stages %s: t=%.3f joins=%llu transport_reads=%llu join_wait_s=%.3f ring_full_waits=%llu ring_full_s=%.3f "
                     "c_ring_full_s=%.3f B_commands=%llu B_idle_s=%.3f B_sleeps=%llu C_commands=%llu C_idle_s=%.3f C_sleeps=%llu\n",
                     label,now_ns()/1e9,(unsigned long long)joins,(unsigned long long)transport_reads,join_wait_ns/1e9,(unsigned long long)space_waits,space_wait_ns/1e9,
                     c_space_wait_ns/1e9,(unsigned long long)b.commands.load(),b.idle_ns.load()/1e9,(unsigned long long)b.sleeps.load(),
                     (unsigned long long)c.commands.load(),c.idle_ns.load()/1e9,(unsigned long long)c.sleeps.load());
    }
    void join() override {
        ++joins;
        if(stats && now_ns()>=next_stats_ns){print_stats("progress");next_stats_ns=now_ns()+2000000000ull;}
        DeviceCall call;call.sync=true;sync_call(call);
        read_cache=read_index.load(std::memory_order_acquire);
        rethrow_failure();
    }
    void post(std::function<void()> work) override {
        const bool due=raster_due;
        auto* call=new DeviceCall;call->work=std::move(work);
        push(DeviceKind::call,0,std::uint64_t(reinterpret_cast<std::uintptr_t>(call)));
        raster_due=due;
    }
    void run_sync(const std::function<void()>& work) override {
        DeviceCall call;call.work=work;call.sync=true;sync_call(call);
        if(call.error)std::rethrow_exception(call.error);
    }
    std::uint32_t read_transport_register(std::uint32_t address,bool reverse) override {
        ++transport_reads;
        std::uint32_t value=0;
        DeviceCall call;call.sync=true;call.transport=true;
        call.work=[&] {
            if(b.failed.load(std::memory_order_acquire))std::rethrow_exception(b.error);
            value=address==0x10003020u?proxy.read_status(vif1.path3_masked,reverse):vif1.read_register(address);
        };
        sync_call(call);
        // A GS failure earlier in the stream, if already reached, is reported first.
        if(c.failed.load(std::memory_order_acquire))std::rethrow_exception(c.error);
        if(call.error)std::rethrow_exception(call.error);
        return value;
    }
    std::exception_ptr stop() override {
        if(!stopped) {
            stopped=true;
            publish_b();
            b.stop(); // Drains B, which publishes everything it forwarded.
            c.stop();
            if(stats)print_stats("final");
        }
        if(c.failed.load(std::memory_order_acquire))return c.error;
        return b.failed.load(std::memory_order_acquire)?b.error:nullptr;
    }
};
}

std::unique_ptr<DeviceLink> start_device_thread(Vif1Path& vif1,GifPath& gif,GsRegisterState& gs,
                                                std::function<void(bool)> context) {
    return std::make_unique<DeviceThread>(vif1,gif,gs,std::move(context));
}
}
