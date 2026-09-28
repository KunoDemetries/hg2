#pragma once
#include "hg/runtime.hpp"
namespace hg {
// Schedule native AOT threads at cooperative boundaries. CPU and guest kernel
// return frames travel together; shared hardware continues while all threads wait.
template<class Run> bool service_ee_thread(State& s,Run run,std::uint64_t quantum=1) {
    if(s.boot.in_interrupt)return false;
    const auto previous=s.boot.current_thread;
    auto chosen=s.boot.threads.size();
    unsigned priority=128;
    if(previous) {
        auto& active=s.boot.threads.at(previous-1);
        if(active.status==1){chosen=previous-1;priority=active.priority;}
    }
    const auto thread_count=s.boot.threads.size();
    auto n=thread_count?s.boot.thread_cursor%thread_count:0;
    for(std::size_t offset=0;offset<thread_count;++offset) {
        const auto& t=s.boot.threads[n];
        if(t.status==2 && t.priority<priority){chosen=n;priority=t.priority;}
        if(++n==thread_count)n=0;
    }
    if(previous && chosen!=previous-1) {
        auto& t=s.boot.threads[previous-1];
        t.context=s.save_cpu();t.kernel_returns=s.boot.returns;t.transfer_pc=s.last_transfer_pc;t.initialized=true;
        if(t.status==1)t.status=2;
        s.boot.current_thread=0;s.boot.returns.clear();
    }
    if(chosen==s.boot.threads.size())return false;
    if(s.boot.current_thread!=chosen+1) {
        auto& t=s.boot.threads[chosen];
        s.restore_cpu(t.context);s.boot.returns=t.kernel_returns;s.last_transfer_pc=t.transfer_pc;
        s.boot.current_thread=std::uint32_t(chosen+1);t.status=1;
    }
    s.boot.threads[chosen].status=1;
    constexpr std::uint32_t completed=0xff003000;
    if(s.pc!=completed)run(s,quantum);
    if(s.pc==completed){s.boot.threads[chosen].status=0x10;s.boot.current_thread=0;}
    return true;
}
}
