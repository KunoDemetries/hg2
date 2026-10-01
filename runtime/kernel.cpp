#include "hg/runtime.hpp"
#include <algorithm>

namespace hg {
static void native_kernel_call(State& s,std::uint32_t number) {
    if (!s.boot.enabled) throw Fault(s.pc,"kernel boundary (native boot services disabled)");
    if(number==0x02) {
        const auto interlace=std::uint32_t(s.r(4)),mode=std::uint32_t(s.r(5)),frame=std::uint32_t(s.r(6));
        if(interlace>1 || frame>1 || mode>0xff)throw Fault(s.pc,"invalid SetGsCrt mode");
        s.gs.crtc_interlace=interlace;s.gs.crtc_mode=mode;s.gs.crtc_frame=frame;
        s.gs.crtc_configured=true;return;
    }
    if (number==0x3c) {
        if(s.boot.thread_ready) throw Fault(s.pc,"SetupThread called twice");
        const auto gp=std::uint32_t(s.r(4)), requested=std::uint32_t(s.r(5));
        const auto size=std::uint32_t(s.r(6)), args=std::uint32_t(s.r(7)), root=std::uint32_t(s.r(8));
        if (!size || size>0x1000000 || size%16 || !root || root%4)
            throw Fault(s.pc,"invalid SetupThread parameters");
        const auto ceiling=s.boot.stack_ceiling;
        if(requested!=0xffffffffu) throw Fault(s.pc,"explicit SetupThread stack needs validation");
        if(size>ceiling || ceiling>s.ram.size() || s.boot.initial_sp<ceiling-size || s.boot.initial_sp>=ceiling)
            throw Fault(s.pc,"invalid native stack profile");
        // argc plus sixteen argv slots, followed by launch-name bytes. This
        // constructs the observed ABI layout; no memory snapshot is embedded.
        const auto text=args+68ull;
        const auto end=text+s.boot.executable_name.size()+1;
        if(args%4 || end>s.ram.size() || end>ceiling-size)
            throw Fault(s.pc,"invalid SetupThread argument block");
        std::fill(s.ram.begin()+args,s.ram.begin()+static_cast<std::size_t>(end),0);
        s.store(args,4,1); s.store(args+4,4,text);
        std::copy(s.boot.executable_name.begin(),s.boot.executable_name.end(),s.ram.begin()+static_cast<std::size_t>(text));
        s.boot.stack_base=ceiling-size; s.boot.root=root; s.boot.gp=gp; s.boot.thread_ready=true;
        s.boot.threads.push_back({1,root,ceiling-size,size,gp,0,0,0,0});
        s.boot.current_thread=1;
        s.w(2,s.boot.initial_sp);
        return;
    }
    if(number==0x3d) {
        if(!s.boot.thread_ready || s.boot.heap_ready) throw Fault(s.pc,"SetupHeap requires one initialized thread");
        const auto start=std::uint32_t(s.r(4)), size=std::uint32_t(s.r(5));
        const std::uint64_t end=size==0xffffffffu ? s.boot.stack_base : std::uint64_t(start)+size;
        if(start%16 || start>=end || end>s.boot.stack_base) throw Fault(s.pc,"invalid SetupHeap bounds");
        s.boot.heap_start=start; s.boot.heap_end=std::uint32_t(end); s.boot.heap_ready=true;
        return; // void ABI: v0 is intentionally not fabricated.
    }
    if(number==0x3e) {
        if(!s.boot.heap_ready) throw Fault(s.pc,"EndOfHeap before SetupHeap");
        s.w(2,s.boot.heap_end); return;
    }
    if(number==0x41 || number==0x42 || number==0x44 || number==0x45) {
        const auto id=std::uint32_t(s.r(4));
        if(!id || id>s.boot.semaphores.size())throw Fault(s.pc,"invalid semaphore identifier");
        auto& semaphore=s.boot.semaphores[id-1];
        if(!semaphore.maximum)throw Fault(s.pc,"deleted semaphore identifier");
        if(number==0x45) {
            if(!semaphore.count){s.w(2,sx32(0xffffffffu));return;}
            // Original USA caller 0x10fa88 compares success directly to its ID.
            --semaphore.count;s.w(2,id);return;
        }
        if(number==0x41) {
            if(s.boot.in_interrupt)throw Fault(s.pc,"DeleteSema in interrupt context");
            semaphore={}; // maximum zero marks a reusable native handle slot.
            s.w(2,0);return;
        }
        if(number==0x42) {
            Thread* waiter=nullptr;
            for(auto& t:s.boot.threads)if((t.status==4 || t.status==0xc) && t.wait_type==3 && t.wait_id==id) {
                if(waiter)throw Fault(s.pc,"multiple EE semaphore waiters require ordering policy");
                waiter=&t;
            }
            if(waiter) {
                waiter->status=waiter->status==0xc?8:2;waiter->wait_type=waiter->wait_id=0;
                waiter->granted_semaphore=id;s.w(2,0);return;
            }
            // EE SignalSema reports failure as -1; a full semaphore cannot
            // accept another credit and retains its current count.
            if(semaphore.count==semaphore.maximum){s.w(2,sx32(0xffffffffu));return;}
            const auto before=semaphore.count;++semaphore.count;
            auto& trace=s.boot.semaphore_trace[s.boot.semaphore_trace_cursor++%s.boot.semaphore_trace.size()];
            trace={'S',id,before,semaphore.count,s.pc,0,s.boot.current_thread,s.boot.in_interrupt,false};
            s.boot.last_semaphore_signal=trace;
        } else {
            if(s.boot.current_thread && s.boot.threads[s.boot.current_thread-1].granted_semaphore==id) {
                s.boot.threads[s.boot.current_thread-1].granted_semaphore=0;s.w(2,0);return;
            }
            if(!semaphore.count)throw Fault(s.pc,"semaphore wait resumed without a signal");
            const auto before=semaphore.count;--semaphore.count;
            auto& trace=s.boot.semaphore_trace[s.boot.semaphore_trace_cursor++%s.boot.semaphore_trace.size()];
            trace={'W',id,before,semaphore.count,s.pc,0,s.boot.current_thread,s.boot.in_interrupt,false};
            s.boot.last_semaphore_wait_success=trace;
        }
        s.w(2,0);return;
    }
    if(number==0x40) {
        const auto address=std::uint32_t(s.r(4));
        if(address%4 || std::uint64_t(address)+24>s.ram.size()) throw Fault(s.pc,"invalid semaphore descriptor");
        const auto maximum=std::uint32_t(s.load(address+4,4,false));
        const auto initial=std::uint32_t(s.load(address+8,4,false));
        const auto attr=std::uint32_t(s.load(address+16,4,false)), option=std::uint32_t(s.load(address+20,4,false));
        if(!maximum || maximum>0x7fffffffu || initial>maximum)
            throw Fault(s.pc,"CreateSema failure ABI not implemented for invalid/exhausted descriptor");
        auto free=std::find_if(s.boot.semaphores.begin(),s.boot.semaphores.end(),[](const Semaphore& item){return !item.maximum;});
        if(free!=s.boot.semaphores.end()) {
            *free={initial,maximum,initial,attr,option};
            s.w(2,std::distance(s.boot.semaphores.begin(),free)+1);
        } else {
            if(s.boot.semaphores.size()>=256)throw Fault(s.pc,"CreateSema exhausted native handles");
            s.boot.semaphores.push_back({initial,maximum,initial,attr,option});
            s.w(2,s.boot.semaphores.size());
        }
        return;
    }
    if(number==0x74) {
        const auto index=std::uint32_t(s.r(4)), target=std::uint32_t(s.r(5));
        if(index>=BootServices::table_entries || target%4 || !target) throw Fault(s.pc,"unsupported SetSyscall entry");
        s.store(BootServices::table+index*4,4,target);
        return;
    }
    if(number==0x64) {
        const auto operation=std::uint32_t(s.r(4));
        if(operation>3) throw Fault(s.pc,"unsupported cache operation");
        // Native guest memory is coherent; completed stores are ordered here.
        // Code changes still require a matching precompiled overlay, never JIT.
        s.sync(0);
        if(operation==0 || operation==1 || operation==3) ++s.boot.data_cache_epoch;
        if(operation==2 || operation==3) ++s.boot.instruction_cache_epoch;
        return;
    }
    if(number==0x10) {
        const auto cause=std::uint32_t(s.r(4)),callback=std::uint32_t(s.r(5));
        if(cause>=15 || !callback || callback%4 || s.boot.interrupt_handlers.size()>=128)
            throw Fault(s.pc,"invalid AddIntcHandler parameters");
        const auto next=std::uint32_t(s.r(6));
        if(s.boot.in_interrupt || (next && next!=0xffffffffu &&
           (next>s.boot.interrupt_handlers.size() ||
            s.boot.interrupt_handlers[next-1].cause!=cause || !s.boot.interrupt_handlers[next-1].callback)))
            throw Fault(s.pc,"invalid AddIntcHandler insertion point or interrupt context");
        s.boot.interrupt_handlers.push_back({cause,callback,s.r(6),s.r(7)});
        s.w(2,s.boot.interrupt_handlers.size());
        return;
    }
    if(number==0x11) {
        const auto cause=std::uint32_t(s.r(4)),id=std::uint32_t(s.r(5));
        if(cause>=15 || !id || id>s.boot.interrupt_handlers.size() ||
           s.boot.interrupt_handlers[id-1].cause!=cause || !s.boot.interrupt_handlers[id-1].callback || s.boot.in_interrupt)
            throw Fault(s.pc,"invalid RemoveIntcHandler request");
        s.boot.interrupt_handlers[id-1].callback=0;s.w(2,0);return;
    }
    if(number==0x4b) {
        s.store(std::uint32_t(s.r(4)),4,s.console.pack()); return;
    }
    if(number==0x13) {
        const auto channel=std::uint32_t(s.r(4)),id=std::uint32_t(s.r(5));
        if(channel>=10 || !id || id>s.boot.dma_handlers.size() || s.boot.dma_handlers[id-1].cause!=channel || !s.boot.dma_handlers[id-1].callback || s.boot.in_interrupt)
            throw Fault(s.pc,"invalid RemoveDmacHandler request");
        s.boot.dma_handlers[id-1].callback=0;s.w(2,0);return;
    }
    if(number==0x12) {
        const auto channel=std::uint32_t(s.r(4)),callback=std::uint32_t(s.r(5));
        if(channel>=10 || !callback || callback%4 || s.boot.dma_handlers.size()>=128)
            throw Fault(s.pc,"invalid or unsupported AddDmacHandler parameters");
        auto free=std::find_if(s.boot.dma_handlers.begin(),s.boot.dma_handlers.end(),[](const InterruptHandler& h){return !h.callback;});
        if(free!=s.boot.dma_handlers.end()){*free={channel,callback,s.r(6),s.r(7)};s.w(2,std::distance(s.boot.dma_handlers.begin(),free)+1);}
        else {s.boot.dma_handlers.push_back({channel,callback,s.r(6),s.r(7)});s.w(2,s.boot.dma_handlers.size());}
        return;
    }
    if(number==0x4a) {
        s.console.unpack(std::uint32_t(s.load(std::uint32_t(s.r(4)),4,false))); return;
    }
    if(number==0x29) {
        auto id=std::uint32_t(s.r(4));
        const auto priority=std::uint32_t(s.r(5));
        if(id==0) id=s.boot.current_thread;
        if(!id || id>s.boot.threads.size() || priority>=128)
            throw Fault(s.pc,"invalid ChangeThreadPriority parameters");
        const auto previous=s.boot.threads[id-1].priority;
        s.boot.threads[id-1].priority=priority;
        // Original 0x1cafa8 saves this result for restoration at 0x1cb030.
        s.w(2,previous); return;
    }
    if(number==0x2b) {
        const auto priority=std::uint32_t(s.r(4));
        if(s.boot.in_interrupt || priority>=128)throw Fault(s.pc,"invalid RotateThreadReadyQueue priority");
        const auto workers=s.boot.threads.size()>1?s.boot.threads.size()-1:0;
        if(s.boot.current_thread && workers) {
            auto& active=s.boot.threads[s.boot.current_thread-1];
            if(active.status==1 && active.priority==priority) {
                active.status=2;
                s.boot.thread_cursor=s.boot.current_thread%std::uint32_t(s.boot.threads.size());
                return;
            }
        }
        if(workers)for(std::size_t offset=0;offset<s.boot.threads.size();++offset) {
            const auto index=(s.boot.thread_cursor+offset)%s.boot.threads.size();
            const auto& thread=s.boot.threads[index];
            if(thread.status==2 && thread.priority==priority) {
                s.boot.thread_cursor=std::uint32_t((index+1)%s.boot.threads.size());break;
            }
        }
        return; // public void ABI
    }
    if(number==0x2f) {
        if(!s.boot.current_thread && !(s.boot.thread_ready && s.boot.in_interrupt))
            throw Fault(s.pc,"GetThreadId without running thread");
        s.w(2,s.boot.current_thread); return;
    }
    if(number==0x30 || number==0x31) {
        auto id=std::uint32_t(s.r(4));const auto address=std::uint32_t(s.r(5));
        if(!id)id=s.boot.current_thread;
        if(!id || id>s.boot.threads.size() || address%4 || std::uint64_t(address)+0x30>s.ram.size())
            throw Fault(s.pc,"invalid ReferThreadStatus request");
        const auto& thread=s.boot.threads[id-1];
        s.store(address+0x00,4,thread.status);s.store(address+0x04,4,thread.entry);
        s.store(address+0x08,4,thread.stack);s.store(address+0x0c,4,thread.stack_size);
        s.store(address+0x10,4,thread.gp);s.store(address+0x14,4,thread.initial_priority);
        s.store(address+0x18,4,thread.priority);s.store(address+0x1c,4,thread.attr);
        s.store(address+0x20,4,thread.option);s.store(address+0x24,4,thread.wait_type);
        s.store(address+0x28,4,thread.wait_id);s.store(address+0x2c,4,thread.wakeup_count);
        s.w(2,0);return;
    }
    if(number==0x32) {
        if(s.boot.in_interrupt || !s.boot.current_thread)throw Fault(s.pc,"SleepThread outside native EE thread");
        auto& t=s.boot.threads[s.boot.current_thread-1];
        if(t.wakeup_count)--t.wakeup_count;else {t.status=4;t.wait_type=1;t.wait_id=0;}
        s.w(2,0);return;
    }
    if(number==0x33 || number==0x133) {
        const bool interrupt_variant=number==0x133;
        const auto id=std::uint32_t(s.r(4));
        if(s.boot.in_interrupt!=interrupt_variant || !id || id>s.boot.threads.size())
            throw Fault(s.pc,"invalid WakeupThread context/identifier");
        auto& thread=s.boot.threads[id-1];
        if(thread.status==4 && thread.wait_type==1) {
            thread.status=2;thread.wait_type=0;thread.wait_id=0;
        } else if(thread.status==0xc && thread.wait_type==1) {
            thread.status=8;thread.wait_type=0;thread.wait_id=0;
        } else if(thread.status==1 || thread.status==2 || thread.status==8 ||
                  thread.status==4 || thread.status==0xc) {
            if(thread.wakeup_count==0xffffffffu)throw Fault(s.pc,"WakeupThread count overflow");
            ++thread.wakeup_count;
        } // Dormant targets are defined to be unaffected.
        s.w(2,0);return;
    }
    if(number==0x35) {
        auto id=std::uint32_t(s.r(4));if(!id)id=s.boot.current_thread;
        if(s.boot.in_interrupt || !id || id>s.boot.threads.size())
            throw Fault(s.pc,"invalid CancelWakeupThread context/identifier");
        auto& thread=s.boot.threads[id-1];const auto previous=thread.wakeup_count;
        thread.wakeup_count=0;s.w(2,previous);return;
    }
    if(number==0x37) {
        auto id=std::uint32_t(s.r(4));if(!id)id=s.boot.current_thread;
        if(s.boot.in_interrupt || !id || id>s.boot.threads.size()) {
            const auto status=id && id<=s.boot.threads.size()?s.boot.threads[id-1].status:0xffffffffu;
            throw Fault(s.pc,"invalid SuspendThread context/identifier id="+std::to_string(id)+
                " count="+std::to_string(s.boot.threads.size())+" status="+std::to_string(status));
        }
        auto& thread=s.boot.threads[id-1];
        // The game's stream bootstrap deliberately probes its newly-created,
        // not-yet-started worker. Dormant is a defined non-mutating failure.
        if(thread.status==0x10){s.w(2,sx32(0xffffffffu));return;}
        if(thread.status==1 || thread.status==2)thread.status=8;
        else if(thread.status==4)thread.status=0xc;
        s.w(2,id);return;
    }
    if(number==0x39) {
        auto id=std::uint32_t(s.r(4));if(!id)id=s.boot.current_thread;
        if(s.boot.in_interrupt || !id || id>s.boot.threads.size() || s.boot.threads[id-1].status==0x10)
            throw Fault(s.pc,"invalid ResumeThread context/identifier");
        auto& thread=s.boot.threads[id-1];
        if(thread.status==8)thread.status=(id==s.boot.current_thread)?1:2;
        else if(thread.status==0xc)thread.status=4;
        s.w(2,id);return;
    }
    if(number==0x20) {
        if(!s.boot.thread_ready) throw Fault(s.pc,"CreateThread before SetupThread");
        const auto address=std::uint32_t(s.r(4));
        auto field=[&](unsigned offset) {return std::uint32_t(s.load(address+offset,4,false));};
        if(address%4 || std::uint64_t(address)+36>s.ram.size()) throw Fault(s.pc,"invalid thread descriptor");
        Thread thread;
        thread.entry=field(4); thread.stack=field(8); thread.stack_size=field(12);
        thread.gp=field(16); thread.initial_priority=thread.priority=field(20);
        thread.attr=field(28); thread.option=field(32);
        if(!thread.entry || thread.entry%4 || thread.entry>=s.ram.size()
            || !thread.stack || thread.stack%16 || thread.stack_size<256
            || thread.stack_size%16 || std::uint64_t(thread.stack)+thread.stack_size>s.ram.size()
            || thread.priority>=128 || s.boot.threads.size()>=255)
            throw Fault(s.pc,"invalid CreateThread parameters or exhausted native thread table");
        s.boot.threads.push_back(thread);
        s.w(2,s.boot.threads.size()); return;
    }
    if(number==0x22) {
        const auto id=std::uint32_t(s.r(4)),argument=std::uint32_t(s.r(5));
        if(s.boot.in_interrupt || !id || id>s.boot.threads.size())
            throw Fault(s.pc,"invalid StartThread context/identifier");
        auto& thread=s.boot.threads[id-1];
        if(thread.status!=0x10 || thread.initialized)
            throw Fault(s.pc,"StartThread requires a dormant new thread");
        constexpr std::uint32_t completed=0xff003000;
        thread.argument=argument;thread.context={};thread.context.pc=thread.entry;
        thread.context.cp0_status=(s.cp0_status|0x10001u)&~6u;
        thread.context.gpr[4].lo=sx32(argument);thread.context.gpr[28].lo=sx32(thread.gp);
        thread.context.gpr[29].lo=sx32(thread.stack+thread.stack_size-16);
        thread.context.gpr[31].lo=sx32(completed);
        thread.initialized=true;thread.status=2;s.w(2,0);return;
    }
    if(number==0x23) {
        if(s.boot.in_interrupt || !s.boot.current_thread || s.boot.current_thread>s.boot.threads.size())
            throw Fault(s.pc,"ExitThread outside a native EE worker");
        s.boot.threads[s.boot.current_thread-1].status=0x10;
        s.boot.current_thread=0;
        // kernel_call advances returning native services by one instruction.
        s.pc=0xff002ffc;return;
    }
    if(number==0x14) {
        const auto cause=std::uint32_t(s.r(4));
        if(cause>=15) throw Fault(s.pc,"invalid EnableIntc cause");
        const auto bit=1u<<cause;
        if(s.intc.mask&bit) throw Fault(s.pc,"EnableIntc already-enabled return ABI needs validation");
        s.intc.write(0x1000f010,bit);
        // First enable observed returning 1 in the USA boot at 0x0026bee8.
        s.w(2,1); return;
    }
    if(number==0x15) {
        const auto cause=std::uint32_t(s.r(4));
        if(cause>=15)throw Fault(s.pc,"invalid DisableIntc cause");
        const auto bit=1u<<cause;
        if(!(s.intc.mask&bit))throw Fault(s.pc,"DisableIntc requires an enabled native cause");
        s.intc.write(0x1000f010,bit);s.w(2,1);return;
    }
    if(number==0x79) {
        const auto id=std::uint32_t(s.r(4)),value=std::uint32_t(s.r(5));
        if(id==1)s.sif->main_address=value;
        else if(id==2)s.sif->sub_address=value;
        else if(id==3)s.sif->main_flags|=value;
        else if(id==4)s.sif->sub_flags&=~value;
        else if(id>=0x80000000u && id<=0x80000002u)s.boot.sif_system_registers[id-0x80000000u]=value;
        else throw Fault(s.pc,"unimplemented hardware SifSetReg operation");
        // Native return policy: submitted value, as for software registers.
        // The original reboot caller ignores hardware-write return values.
        s.w(2,sx32(value));return;
    }
    if(number==0x7f) {
        s.w(2,s.ram.size());return;
    }
    if(number==0x7c) {
        const auto operation=std::uint32_t(s.r(4)),arguments=std::uint32_t(s.r(5));
        if(operation!=0x10 || arguments%4 || std::uint64_t(physical_address(arguments))+4>s.ram.size())
            throw Fault(s.pc,"unsupported Deci2Call operation/request");
        const auto message=std::uint32_t(s.load(arguments,4,false));
        std::string text;
        for(std::uint32_t n=0;n<4096;++n) {
            const auto address=std::uint64_t(physical_address(message))+n;
            if(address>=s.ram.size())throw Fault(s.pc,"DECI2 kputs string outside EE RAM");
            const auto byte=char(s.load(message+n,1,false));
            if(!byte)break;
            text.push_back(byte);
            if(n==4095)throw Fault(s.pc,"unterminated DECI2 kputs string");
        }
        if(s.boot.deci2_output.size()+text.size()>65536)throw Fault(s.pc,"DECI2 diagnostic output exceeds bounded history");
        s.boot.deci2_output+=text;++s.boot.deci2_kputs_calls;
        // The game ignores this value; retain a positive successful-call ABI
        // until an original-hardware observation establishes finer behavior.
        s.w(2,1);return;
    }
    if(number==0x6b) {
        // Native idle-boundary SifStopDma. Mid-packet cancellation needs its
        // own verified FIFO policy; never discard an in-flight payload.
        auto& receive=s.dmac.channels[5];
        if(receive.qwc || s.dmac.sif0_packet)
            throw Fault(s.pc,"SifStopDma during an active receive packet");
        receive.chcr&=~0x100u;
        return; // void ABI
    }
    if(number==0x70) {
        s.w(2,s.gs_imr());return;
    }
    if(number==0x71) {
        s.gs_put_imr(s.r(4));return; // public void ABI
    }
    if(number==0x76) {
        const auto id=std::uint32_t(s.r(4));
        if(!id || id>s.boot.sif_transfer_id)throw Fault(s.pc,"invalid SifDmaStat transfer identifier");
        const bool active=id==s.boot.active_sif_transfer &&
            ((s.dmac.channels[6].chcr&0x100) || s.sif->pending_sub());
        if(active)s.w(2,0);
        else {
            if(id==s.boot.active_sif_transfer)s.boot.active_sif_transfer=0;
            s.w(2,sx32(0xffffffffu));
        }
        return;
    }
    if(number==0x77) {
        // Native kernel staging area, below the cleared kernel boundary.
        constexpr std::uint32_t pool=0x60000,pool_end=0x80000;
        constexpr std::uint32_t packet_capacity=pool_end-pool-16;
        const auto descriptor=std::uint32_t(s.r(4)),count=std::uint32_t(s.r(5));
        if((count!=1 && count!=2) || descriptor%4 || std::uint64_t(descriptor)+count*16>0x100000000ull)
            throw Fault(s.pc,"SifSetDma supports one or two bounded descriptors");
        std::vector<std::uint8_t> packet;
        BootServices::SifDmaTrace trace{};trace.pc=s.pc;trace.count=count;
        auto append_word=[&](std::uint32_t value){for(unsigned byte=0;byte<4;++byte)packet.push_back(std::uint8_t(value>>(byte*8)));};
        for(unsigned index=0;index<count;++index) {
            const auto entry=descriptor+index*16;
            const auto source=std::uint32_t(s.load(entry,4,false)),destination=std::uint32_t(s.load(entry+4,4,false));
            const auto size=std::uint32_t(s.load(entry+8,4,false)),attr=std::uint32_t(s.load(entry+12,4,false));
            trace.descriptors[index]={source,destination,size,attr};
            // Public SIF DMA attributes identify the two independently observed
            // profiles: raw copies use no flags, while command packets request
            // remote termination plus an output interrupt (0x40 | 0x04).  A
            // terminal descriptor must be last because the IOP destination
            // channel is no longer active after it completes.
            const bool terminal=(attr&0x40u)!=0,interrupt_output=(attr&0x04u)!=0;
            if(source%16 || destination%4 ||
               (attr==0 ? (!size || size%4 || size>packet_capacity-16) :
                attr==0x44 ? (size<16 || size>112) : true) ||
               (terminal && index+1!=count))
                throw Fault(s.pc,"SifSetDma descriptor outside native RPC profile");
            const auto rounded=(size+15)&~15u;
            if(packet.size()+16+rounded>packet_capacity)
                throw Fault(s.pc,"SifSetDma exceeds native kernel staging capacity");
            if(std::uint64_t(destination)+rounded>0x200000 || std::uint64_t(source)+rounded>0x100000000ull)
                throw Fault(s.pc,"SifSetDma payload outside address space");
            // The IOP destination tag carries the requested remote lifecycle.
            // A raw transfer leaves that receiver armed for later SIF traffic;
            // the EE source-chain transfer still completes independently.
            append_word(destination|(terminal?0x80000000u:0)|(interrupt_output?0x40000000u:0));
            append_word(rounded/4);append_word(0);append_word(0);
            for(unsigned n=0;n<rounded;++n)packet.push_back(std::uint8_t(s.load(source+n,1,false)));
        }
        // Validate/snapshot every descriptor before any guest-visible mutation.
        s.boot.sif_dma_trace[s.boot.sif_dma_trace_cursor++%s.boot.sif_dma_trace.size()]=trace;
        if((s.dmac.channels[6].chcr&0x100) || s.sif->pending_sub()){s.w(2,0);return;}
        if(s.boot.sif_transfer_id==0x7fffffffu)throw Fault(s.pc,"native SIF transfer identifiers exhausted");
        s.store(pool,8,(std::uint64_t(pool+16)<<32)|(packet.size()/16));s.store(pool+8,8,0);
        std::copy(packet.begin(),packet.end(),s.ram.begin()+pool+16);
        s.store(0x1000c430,4,pool);s.store(0x1000c400,4,0x184);
        s.boot.active_sif_transfer=++s.boot.sif_transfer_id;
        s.w(2,s.boot.active_sif_transfer);return;
    }
    if(number==0x7a) {
        const auto id=std::uint32_t(s.r(4));
        std::uint32_t value;
        if(id>=0x80000000u && id<=0x80000002u)value=s.boot.sif_system_registers[id-0x80000000u];
        else if(id==1)value=s.sif->main_address;
        else if(id==2)value=s.sif->sub_address;
        else if(id==3)value=s.sif->main_flags;
        else if(id==4)value=s.sif->sub_flags;
        else throw Fault(s.pc,"unsupported SifGetReg identifier");
        s.w(2,sx32(value));return;
    }
    if(number==0x78) {
        // Public void ABI: arm SIF0 destination-chain reception with tag IRQs.
        // Completion waits for actual peripheral input; no packet is fabricated.
        s.store(0x1000c000,4,(1u<<2)|(1u<<7)|(1u<<8));return;
    }
    if(number==0x17) {
        const auto channel=std::uint32_t(s.r(4));
        if(channel>=10 || !(s.dmac.mask&(1u<<channel)))throw Fault(s.pc,"DisableDmac requires an enabled native channel");
        s.store(0x1000e010,4,1u<<(channel+16));s.w(2,1);return;
    }
    if(number==0x16) {
        const auto channel=std::uint32_t(s.r(4));
        if(channel>=10)throw Fault(s.pc,"unsupported EnableDmac channel");
        const auto bit=1u<<channel;
        if(s.dmac.mask&bit)throw Fault(s.pc,"EnableDmac already-enabled return ABI needs validation");
        s.store(0x1000e010,4,bit<<16);
        // Original first enable returns 1 through the game's wrapper at 0x26f1a0.
        s.w(2,1);return;
    }
    throw Fault(s.pc,"unimplemented kernel syscall " + std::to_string(number));
}
void kernel_call(State& s) {
    if(!s.boot.enabled) throw Fault(s.pc,"kernel boundary (native boot services disabled)");
    if(!s.boot.table_ready) {
        // Explicit native compatibility data, not BIOS bytes. The game's own
        // initialization locates this table using its registered helper pointers.
        s.boot.table_ready=true;
        std::fill(s.ram.begin(),s.ram.begin()+0x80000,0);
        for(unsigned i=0;i<BootServices::table_entries;++i) s.store(BootServices::table+i*4,4,BootServices::gateway+i*4);
    }
    auto number=std::uint32_t(s.r(3));
    if(number==0xffffff88u && s.boot.in_interrupt)number=0x78;
    if(number==0xffffff89u && s.boot.in_interrupt)number=0x77;
    if(number==0xffffff8au && s.boot.in_interrupt)number=0x76;
    if(number==0xffffffbdu && s.boot.in_interrupt)number=0x42;
    if(number==0xffffffd1u && s.boot.in_interrupt)number=0x2f;
    if(number==0xffffffcfu && s.boot.in_interrupt)number=0x31;
    if(number==0xffffffccu && s.boot.in_interrupt)number=0x133;
    if(number>=BootServices::table_entries) throw Fault(s.pc,"interrupt/negative syscall ABI not yet implemented");
    const auto handler=std::uint32_t(s.load(BootServices::table+number*4,4,false));
    if(handler==BootServices::gateway+number*4) {
        if(number==0x44) {
            const auto id=std::uint32_t(s.r(4));
            if(s.boot.in_interrupt || !id || id>s.boot.semaphores.size())throw Fault(s.pc,"invalid WaitSema context/identifier");
            // A single cooperative caller remains at its syscall until a real
            // signal arrives. The host continues peripheral/interrupt servicing.
            if(!s.boot.semaphores[id-1].maximum)throw Fault(s.pc,"WaitSema on deleted semaphore");
            const bool granted=s.boot.current_thread && s.boot.threads[s.boot.current_thread-1].granted_semaphore==id;
            if(!s.boot.semaphores[id-1].count && !granted) {
                if(s.boot.current_thread) {
                    auto& t=s.boot.threads[s.boot.current_thread-1];t.status=4;t.wait_type=3;t.wait_id=id;
                }
                auto& trace=s.boot.semaphore_trace[s.boot.semaphore_trace_cursor++%s.boot.semaphore_trace.size()];
                trace={'W',id,0,0,s.pc,handler,s.boot.current_thread,s.boot.in_interrupt,true};
                return;
            }
        }
        native_kernel_call(s,number);
        s.pc+=4;
    } else {
        if(!handler || handler%4) throw Fault(s.pc,"invalid guest syscall handler");
        s.boot.returns.push_back({s.pc+4,s.r(31)});
        s.w(31,BootServices::return_gateway);
        s.pc=handler;
    }
}
bool return_from_kernel(State& s) {
    if(s.pc!=BootServices::return_gateway) return false;
    if(s.boot.returns.empty()) throw Fault(s.pc,"guest kernel return without caller");
    const auto frame=s.boot.returns.back(); s.boot.returns.pop_back();
    s.pc=frame.pc; s.w(31,frame.ra); return true;
}
}
