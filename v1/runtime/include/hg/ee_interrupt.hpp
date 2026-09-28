#pragma once
#include "hg/runtime.hpp"
#include <algorithm>
namespace hg {
// Native DMA callback dispatch at cooperative boundaries. This is not a complete
// EE exception controller; unimplemented sources and handler chains stay explicit.
class EeDmaInterruptDispatcher {
    State* owner=nullptr;
    EeCpuContext saved{};
    std::uint32_t transfer=0;
    std::size_t return_depth=0;
    unsigned steps=0;
public:
    bool active() const {return owner!=nullptr;}
    template<class Run>
    bool service(State& s,Run run,unsigned quantum=1) {
        constexpr std::uint32_t return_pc=0xff002000;
        if(owner && owner!=&s)throw Fault(s.pc,"EE DMA dispatcher state changed while active");
        if(!quantum)throw Fault(s.pc,"EE DMA callback requires a nonzero quantum");
        if(!owner) {
            if(s.boot.in_interrupt || (s.cp0_status&0x10007u)!=0x10001u)return false;
            if(!(s.dmac.status&s.dmac.mask&0x20))return false;
            const InterruptHandler* selected=nullptr;
            for(const auto& h:s.boot.dma_handlers)if(h.cause==5 && h.callback) {
                if(selected)throw Fault(s.pc,"multiple EE DMA callbacks require ordered-chain dispatch");
                selected=&h;
            }
            if(!selected)throw Fault(s.pc,"EE SIF0 interrupt has no registered callback");
            const auto handler=*selected;
            saved=s.save_cpu();transfer=s.last_transfer_pc;return_depth=s.boot.returns.size();
            owner=&s;steps=0;
            s.dmac.status&=~0x20u;s.boot.in_interrupt=true;s.cp0_status&=~0x10001u;
            s.pc=handler.callback;s.w(4,5);s.w(5,handler.arg);s.w(6,0);
            s.w(29,0x5f000);s.w(31,return_pc);
        }
        for(unsigned n=0;n<quantum && s.pc!=return_pc;++n) {
            if(steps==10000)throw Fault(s.pc,"EE DMA callback exceeded instruction budget");
            ++steps;run(s,1);
        }
        if(s.pc!=return_pc)return true;
        if(s.r(2)!=0 || s.boot.returns.size()!=return_depth)
            throw Fault(s.pc,"unsupported EE DMA callback return or outstanding kernel frame");
        s.restore_cpu(saved);s.last_transfer_pc=transfer;
        s.boot.in_interrupt=false;owner=nullptr;return true;
    }
};
// Cooperative INTC delivery with stable handler IDs and registration order.
// next=0 inserts at the front, -1 at the back, or before an existing ID.
class EeIntcInterruptDispatcher {
    State* owner=nullptr;
    EeCpuContext saved{};
    std::uint32_t transfer=0,cause=0;
    std::size_t return_depth=0,index=0;
    unsigned steps=0;
    std::vector<InterruptHandler> handlers;
    static constexpr std::uint32_t return_pc=0xff002100;
    void enter(State& s) {
        s.restore_cpu(saved);s.last_transfer_pc=transfer;
        s.boot.in_interrupt=true;s.cp0_status&=~0x10001u;
        s.pc=handlers[index].callback;s.w(4,cause);s.w(5,handlers[index].arg);s.w(6,0);
        s.w(29,0x5f000);s.w(31,return_pc);
    }
public:
    bool active() const {return owner!=nullptr;}
    template<class Run> bool service(State& s,Run run,unsigned quantum=1) {
        if(owner && owner!=&s)throw Fault(s.pc,"EE INTC dispatcher state changed while active");
        if(!quantum)throw Fault(s.pc,"EE INTC callback requires a nonzero quantum");
        if(!owner) {
            if(s.boot.in_interrupt || (s.cp0_status&0x10007u)!=0x10001u)return false;
            const auto pending=s.intc.status&s.intc.mask;
            if(!pending)return false;
            cause=0;while(!(pending&(1u<<cause)))++cause;
            std::vector<std::size_t> order;
            // Retain removed nodes during reconstruction so later insertions
            // relative to their formerly-live IDs keep their established order.
            for(std::size_t n=0;n<s.boot.interrupt_handlers.size();++n) {
                const auto& h=s.boot.interrupt_handlers[n];if(h.cause!=cause)continue;
                const auto next=std::uint32_t(h.next);
                if(!next)order.insert(order.begin(),n);
                else if(next==0xffffffffu)order.push_back(n);
                else {
                    auto pos=std::find(order.begin(),order.end(),std::size_t(next-1));
                    if(pos==order.end())throw Fault(s.pc,"invalid EE INTC handler chain");
                    order.insert(pos,n);
                }
            }
            handlers.clear();for(auto n:order)if(s.boot.interrupt_handlers[n].callback)
                handlers.push_back(s.boot.interrupt_handlers[n]);
            if(handlers.empty())throw Fault(s.pc,"EE INTC interrupt has no registered callback");
            saved=s.save_cpu();transfer=s.last_transfer_pc;return_depth=s.boot.returns.size();
            owner=&s;index=0;steps=0;
            s.intc.write(0x1000f000,1u<<cause);enter(s);
        }
        for(unsigned n=0;n<quantum;++n) {
            if(steps++==100000)throw Fault(s.pc,"EE INTC chain exceeded instruction budget");
            run(s,1);
            if(s.pc!=return_pc)continue;
            if(s.r(2)!=0 || s.boot.returns.size()!=return_depth)
                throw Fault(s.pc,"unsupported EE INTC callback return or outstanding kernel frame");
            if(++index<handlers.size()){enter(s);continue;}
            s.restore_cpu(saved);s.last_transfer_pc=transfer;s.boot.in_interrupt=false;owner=nullptr;break;
        }
        return true;
    }
};
// Synchronous convenience wrapper for hosts without another CPU to interleave.
template<class Run>
bool service_ee_dma_interrupt(State& s,Run run) {
    EeDmaInterruptDispatcher dispatcher;
    const bool serviced=dispatcher.service(s,run,10000);
    if(dispatcher.active())throw Fault(s.pc,"EE DMA callback exceeded instruction budget");
    return serviced;
}
}
