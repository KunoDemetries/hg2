#pragma once
#include "hg/runtime.hpp"
#include "hg/iop.hpp"

namespace hg {
inline void initialize_sif_transport(State& ee,IopState& iop) {
    if(ee.sif!=iop.sif || (ee.dmac.channels[5].chcr&0x1cc)!=0x184)
        throw std::runtime_error("SIF initialization requires connected endpoints and armed EE receive chain");
    // Native platform boot service: both FIFO directions have real producers and
    // consumers. SIFCMD publishes its own later readiness and buffer address.
    ee.sif->main_flags|=0x10000;
}
// Cooperative transport service; instruction timing and interrupt dispatch are
// deliberately separate. Never consume input while the EE receiver is disarmed.
inline bool service_sif(State& ee,IopState& iop) {
    if(ee.sif!=iop.sif)throw std::runtime_error("EE and IOP must share a SIF link");
    bool progress=iop.pump_sif0();
    progress=ee.dmac.pump_sif1([&](std::uint32_t address) {
        const bool spr=(address&0x80000000)!=0;const auto offset=address&0x7fffffff;
        const auto size=spr?ee.scratch.size():ee.ram.size();
        if(offset%16 || std::uint64_t(offset)+16>size)throw Fault(ee.pc,"SIF1 source outside implemented memory");
        const auto* p=(spr?ee.scratch.data():ee.ram.data())+offset;
        SifLink::Quad q;
        for(unsigned n=0;n<8;++n){q.low|=std::uint64_t(p[n])<<(n*8);q.high|=std::uint64_t(p[n+8])<<(n*8);}
        return q;
    },[&](SifLink::Quad quad){return ee.sif->push_sub(quad);}) || progress;
    if(ee.dmac.enabled(5) && (ee.dmac.channels[5].chcr&0x100) && ee.sif->peek_main()) {
        const auto quad=*ee.sif->peek_main();
        ee.receive_sif0(quad.low,quad.high);
        ee.sif->consume_main();progress=true;
    }
    if((iop.dmac.channel[2][2]&0x01000000) && ee.sif->peek_sub()) {
        const auto quad=*ee.sif->peek_sub();
        iop.dmac.receive_sif1(quad.low,quad.high,[&](std::uint32_t address,std::uint64_t low,std::uint64_t high) {
            if(address%4 || std::uint64_t(address)+16>iop.ram.size())throw IopFault(iop.pc,"SIF1 destination outside IOP RAM");
            for(unsigned n=0;n<8;++n){iop.ram[address+n]=std::uint8_t(low>>(n*8));iop.ram[address+n+8]=std::uint8_t(high>>(n*8));}
        });
        ee.sif->consume_sub();progress=true;
    }
    return progress;
}
}
