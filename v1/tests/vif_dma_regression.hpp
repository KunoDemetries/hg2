#pragma once
#include "hg/runtime.hpp"
#include "hg/vif_dma.hpp"
#include "vif_direct_regression.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <string>

namespace vif_dma_regression {
inline bool same(const hg::State& a,const hg::State& b) {
    for(unsigned i=0;i<a.dmac.channels.size();++i) {
        const auto& x=a.dmac.channels[i];const auto& y=b.dmac.channels[i];
        if(x.chcr!=y.chcr||x.madr!=y.madr||x.qwc!=y.qwc||x.tadr!=y.tadr||
           x.asr0!=y.asr0||x.asr1!=y.asr1||x.sadr!=y.sadr)return false;
    }
    return a.dmac.status==b.dmac.status&&a.dmac.mask==b.dmac.mask&&
        a.dmac.control==b.dmac.control&&a.dmac.priority==b.dmac.priority&&a.dmac.held==b.dmac.held&&
        a.dmac.vif1_packet==b.dmac.vif1_packet&&a.dmac.vif1_end==b.dmac.vif1_end&&
        a.intc.status==b.intc.status&&a.intc.mask==b.intc.mask&&a.pc==b.pc&&
        vif_direct_regression::same_vif(a.vif1,b.vif1)&&a.gif.pending==b.gif.pending&&
        gif_stream_regression::same_state(a.gs,b.gs);
}
inline void word(std::vector<std::uint8_t>& data,std::size_t at,std::uint32_t value) {
    for(unsigned n=0;n<4;++n)data.at(at+n)=std::uint8_t(value>>(8*n));
}
inline void reset(hg::State& s,const hg::GsRegisterState& gs,unsigned units,bool chain=false,bool scratch=false) {
    s.dmac={};s.intc={};s.vif1=hg::Vif1Path{};s.gif.pending.clear();s.gs=gs;s.pc=0x100220;
    auto& c=s.dmac.channels[1];c.chcr=chain?0x105:0x101;c.qwc=units;c.madr=scratch?0x80000100u:0x1000u;
    c.tadr=0x2000;c.asr0=0x3000;c.asr1=0x4000;
    s.dmac.vif1_packet=chain;s.dmac.vif1_end=true;s.dmac.status=0x40;
    s.intc.status=0x100;s.intc.mask=0x7fff;
    s.vif1.pending.resize(4);word(s.vif1.pending,0,0x50000000u|(units&0xffff));s.vif1.pending_phase=12;
    s.vif1.vu_mem[3]=0x11223344;s.vif1.vu1.issue_cycle=12345;
}
inline int run() {
    auto a=std::make_unique<hg::State>(),b=std::make_unique<hg::State>();
    const auto initial=gif_stream_regression::initial_state();
    // Source is a stream of empty EOP GIF packets, each a defined no-draw packet.
    for(std::size_t i=0x1000;i<0x101100;i+=16){a->ram[i+1]=b->ram[i+1]=0x80;}
    for(std::size_t i=0;i<a->scratch.size();i+=16){a->scratch[i+1]=b->scratch[i+1]=0x80;}
    std::uint64_t cases=0,accepted=0,scalar_steps=0;
    for(unsigned seed=0;seed<2048;++seed) {
        const unsigned units=4+((seed*37u)%509);
        const bool chain=(seed&16)!=0,scratch=(seed&32)!=0;
        reset(*a,initial,units,chain,scratch);reset(*b,initial,units,chain,scratch);
        const auto alter=[&](hg::State& s) {
            auto& c=s.dmac.channels[1];
            s.vif1.pending.resize(4+(seed%7));
            if(seed&64){s.gs.privileged_csr_events=2;s.gs.privileged_imr=0;}
            switch(seed%16) {
            case 0:break;
            case 1:s.dmac.held=true;break;
            case 2:c.chcr&=~0x100u;break;
            case 3:c.qwc=seed%3;break;
            case 4:c.chcr=(c.chcr&~0xcu)|8;break;
            case 5:c.chcr=(c.chcr&~0xcu)|4;s.dmac.vif1_packet=false;break;
            case 6:s.vif1.pending.resize(seed%4);break;
            case 7:s.vif1.pending[3]=0xd0;break;
            case 8:s.vif1.pending_phase=seed%11;break;
            case 9:s.vif1.pending.resize(4+units*16);break;
            case 10:++c.madr;break;
            case 11:c.madr=scratch?0x80004000u:0x02000000u;break;
            case 12:c.qwc=0xffffffffu;break;
            case 13:c.madr=scratch?0x80003fd0u:0x01ffffd0u;break;
            case 14:s.vif1.pending[3]=0x51;break;
            case 15:s.dmac.priority=0x80000000u;break;
            }
        };
        alter(*a);alter(*b);++cases;
        const auto count=hg::buffer_incomplete_vif_direct(*a);
        if(count) {
            ++accepted;scalar_steps+=count;
            if(count<2||!a->dmac.channels[1].qwc){std::cerr<<"VIF inert boundary crossed\n";return 1;}
            for(std::uint32_t n=0;n<count;++n)if(!b->pump_vif1())return 1;
        }
        if(!same(*a,*b)) {std::cerr<<"VIF inert batch mismatch seed="<<seed<<" qwords="<<count<<'\n';return 1;}
    }
    // The caller may avoid invoking the helper for other command bytes: every
    // skipped invocation must be a state-preserving zero result, including IRQ codes.
    for(unsigned command=0;command<256;++command) {
        reset(*a,initial,128);reset(*b,initial,128);
        a->vif1.pending[3]=b->vif1.pending[3]=std::uint8_t(command);
        const auto unconditional=hg::buffer_incomplete_vif_direct(*a);
        std::uint32_t conditional=0;
        if(b->vif1.pending.size()>=4&&(b->vif1.pending[3]&0xfeu)==0x50u)
            conditional=hg::buffer_incomplete_vif_direct(*b);
        ++cases;
        if(unconditional!=conditional||!same(*a,*b)) {
            std::cerr<<"VIF inert caller filter mismatch command="<<command<<'\n';return 1;
        }
    }
    // Complete command/DMA and first faults still execute through the original
    // pump. Compare the whole unchanged service loop with a batched loop.
    for(bool chain:{false,true})for(bool scratch:{false,true})for(unsigned variant=0;variant<9;++variant) {
        const unsigned units=variant==8?65536u:128u;
        if(scratch&&variant==8)continue;
        reset(*a,initial,units,chain,scratch);reset(*b,initial,units,chain,scratch);
        const auto alter=[&](hg::State& s) {
            auto& c=s.dmac.channels[1];
            switch(variant) {
            case 0:break;
            case 1:c.madr=scratch?0x80003fd0u:0x01ffffd0u;break;
            case 2:s.vif1.pending_phase=0;break;
            case 3:s.vif1.pending[3]=0xd1;break;
            case 4:s.dmac.held=true;break;
            case 5:s.dmac.control=0;break;
            case 6:s.gs.privileged_csr_events=2;s.gs.privileged_imr=0;break;
            case 7: {
                // Last GIF packet fails at the final DMA qword. Its exact partial
                // state and QWC/MADR before the rejected sink must be preserved.
                auto* src=(scratch?s.scratch.data():s.ram.data())+(scratch?0x100:0x1000);
                const auto tag=gif_stream_regression::tag(0,1,1,false,true);
                const std::size_t at=(units-2)*16;
                for(unsigned j=0;j<8;++j)src[at+j]=std::uint8_t(tag>>(8*j));
                for(unsigned j=8;j<16;++j)src[at+j]=0;
                src[at+8]=14;
                for(unsigned j=0;j<16;++j)src[at+16+j]=0;
                src[at+24]=0x80;break;
            }
            case 8:break;
            }
        };
        alter(*a);alter(*b);++cases;
        const auto fast_fault=gif_stream_regression::fault([&] {
            unsigned limit=0;
            while(a->pump_vif1()) {
                if(++limit>70000)throw std::runtime_error("synthetic DMA runaway");
                if(a->vif1.pending.size()>=4&&(a->vif1.pending[3]&0xfeu)==0x50u)
                    scalar_steps+=hg::buffer_incomplete_vif_direct(*a);
            }
        });
        const auto ref_fault=gif_stream_regression::fault([&] {
            unsigned limit=0;while(b->pump_vif1())if(++limit>70000)throw std::runtime_error("synthetic DMA runaway");
        });
        if(fast_fault!=ref_fault||!same(*a,*b)) {
            std::cerr<<"VIF inert full burst mismatch variant="<<variant<<" chain="<<chain<<" scratch="<<scratch
                     <<" actual='"<<fast_fault<<"' expected='"<<ref_fault<<"'\n";return 1;
        }
        // Restore the locally edited fault packet in both source memories.
        if(variant==7)for(auto* s:{a.get(),b.get()}) {
            auto* src=(scratch?s->scratch.data():s->ram.data())+(scratch?0x100:0x1000);
            for(unsigned unit=units-2;unit<units;++unit) {
                for(unsigned j=0;j<16;++j)src[unit*16+j]=0;
                src[unit*16+1]=0x80;
            }
        }
    }
    if(!accepted||a->ram!=b->ram||a->scratch!=b->scratch)return 1;
    std::cout<<"VIF inert DMA: "<<cases<<" exact state/fault cases, "<<accepted
             <<" accepted predicate cases / "<<scalar_steps<<" reference-equivalent qwords passed\n";
    return 0;
}
}
