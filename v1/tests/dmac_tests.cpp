#include "hg/runtime.hpp"
#include "hg/gs.hpp"
#include "hg/iop.hpp"
#include "hg/system.hpp"
#include <iostream>
#define CHECK(x) do {if(!(x)){std::cerr<<"Failed line "<<__LINE__<<": "<<#x<<'\n';return 1;}}while(0)
int main() {
    // fromIPU blocks without output, writes real qwords to RAM/scratchpad,
    // and preserves the FIFO when destination validation fails.
    {
        auto storage=std::make_unique<hg::State>();auto& s=*storage;auto& c=s.dmac.channels[3];
        s.store(0x1000b010,4,0x80001800);s.store(0x1000b020,4,2);s.store(0x1000b000,4,0x100);
        CHECK(!s.pump_ipu_output() && c.qwc==2 && (c.chcr&0x100));
        CHECK(s.ipu.push_output({0x1122334455667788ull,0x99aabbccddeeff00ull}));
        s.dmac.held=true;CHECK(!s.pump_ipu_output() && s.ipu.output_count==1);s.dmac.held=false;
        CHECK(s.pump_ipu_output() && c.qwc==1 && c.madr==0x80001810 && !(s.dmac.status&8));
        CHECK(s.load(0x70001800,8,false)==0x1122334455667788ull && s.load(0x70001808,8,false)==0x99aabbccddeeff00ull);
        CHECK(s.ipu.push_output({7,9}));CHECK(s.pump_ipu_output() && c.qwc==0 && !(c.chcr&0x100) && (s.dmac.status&8));
        for(unsigned n=0;n<8;++n)CHECK(s.ipu.push_output({n+10,n+20}));CHECK(!s.ipu.push_output({0,0}) && ((s.ipu.control()>>4)&15)==8);
        s.store(0x1000b010,4,0x4000);s.store(0x1000b020,4,8);s.store(0x1000b000,4,0x100);
        for(unsigned n=0;n<8;++n){CHECK(s.pump_ipu_output());CHECK(s.load(0x4000+n*16,8,false)==n+10 && s.load(0x4008+n*16,8,false)==n+20);}
        s.store(0x1000b010,4,0x80004000);s.store(0x1000b020,4,1);s.store(0x1000b000,4,0x100);s.ipu.push_output({1,2});
        bool rejected=false;try{s.pump_ipu_output();}catch(const hg::Fault&){rejected=true;}CHECK(rejected && s.ipu.output_count==1 && c.qwc==1);
        s.dmac.held=true;s.store(0x1000b000,4,0);s.dmac.held=false;
        rejected=false;try{s.store(0x1000b000,4,0x104);}catch(const hg::Fault&){rejected=true;}CHECK(rejected && !(c.chcr&0x100));
    }
    // toIPU retains every accepted qword, stalls at eight, and resumes after
    // an actual table command consumes input. Completion is not FIFO drain.
    {
        auto storage=std::make_unique<hg::State>();auto& s=*storage;
        for(unsigned n=0;n<10;++n){s.store(0x4000+n*16,8,0x12340000+n);s.store(0x4008+n*16,8,0x56780000+n);}
        s.store(0x1000b410,4,0x4000);s.store(0x1000b420,4,10);s.store(0x1000b400,4,0x101);
        s.dmac.held=true;CHECK(!s.pump_ipu_input() && s.ipu.input_count==0);s.dmac.held=false;
        for(unsigned n=0;n<8;++n)CHECK(s.pump_ipu_input());
        auto& c=s.dmac.channels[4];
        CHECK(s.ipu.input_count==8 && c.qwc==2 && c.madr==0x4080 && (c.chcr&0x100));
        CHECK(!s.pump_ipu_input() && c.qwc==2 && !(s.dmac.status&16));
        s.ipu.command(0x50000000);CHECK(s.ipu.intra_iq[3].low==0x12340003);
        CHECK(s.pump_ipu_input() && s.pump_ipu_input());
        CHECK(c.qwc==0 && c.madr==0x40a0 && !(c.chcr&0x100) && (s.dmac.status&16));
        CHECK(s.ipu.buffered_count==1 && s.ipu.buffered[0].low==0x12340004 && s.ipu.input_count==5 && s.ipu.input[4].high==0x56780009);
        // Existing partial REF data must precede the next REFE packet.
        s.ipu.reset();s.dmac.status=0;
        s.store(0x3000,8,(0x4020ull<<32)|2);s.store(0x3008,8,0);
        s.store(0x1000b410,4,0x4000);s.store(0x1000b420,4,2);s.store(0x1000b430,4,0x3000);
        s.store(0x1000b400,4,0x30000105);
        CHECK(s.pump_ipu_input() && s.pump_ipu_input());
        CHECK(c.tadr==0x3000 && (c.chcr&0x100) && !(s.dmac.status&16));
        CHECK(s.pump_ipu_input() && s.pump_ipu_input());
        CHECK(c.tadr==0x3010 && !(c.chcr&0x100) && s.ipu.input_count==4);
        for(unsigned n=0;n<4;++n)CHECK(s.ipu.input[n].low==0x12340000+n);
        // A fresh NEXT chain skips the intervening tag; IRQ+TIE ends a REF.
        s.ipu.reset();s.dmac.status=0;
        s.store(0x3000,8,(0x3040ull<<32)|0x20000001u);s.store(0x3010,8,0x111);s.store(0x3018,8,0x222);
        s.store(0x3040,8,(0x4000ull<<32)|0xb0000001u);s.store(0x3048,8,0);
        s.store(0x1000b420,4,0);s.store(0x1000b430,4,0x3000);s.store(0x1000b400,4,0x185);
        CHECK(s.pump_ipu_input() && c.tadr==0x3040 && !(s.dmac.status&16));
        CHECK(s.pump_ipu_input() && (s.dmac.status&16) && !(c.chcr&0x100));
        CHECK(s.ipu.input[0].low==0x111 && s.ipu.input[1].low==0x12340000);
        // Unsupported control and invalid source spans remain explicit faults.
        for(unsigned mode:{0x109u,0x145u,0x115u}) {
            bool rejected=false;try{s.store(0x1000b400,4,mode);}catch(const hg::Fault&){rejected=true;}
            CHECK(rejected && !(c.chcr&0x100));
        }
        s.ipu.reset();s.store(0x1000b410,4,std::uint32_t(s.ram.size()));s.store(0x1000b420,4,1);s.store(0x1000b400,4,0x101);
        bool rejected=false;try{s.pump_ipu_input();}catch(const hg::Fault&){rejected=true;}
        CHECK(rejected && c.qwc==1 && s.ipu.input_count==0);
        s.dmac.held=true;s.store(0x1000b400,4,0);s.dmac.held=false;
        s.store(0x3000,8,0x40000001);s.store(0x1000b420,4,0);s.store(0x1000b430,4,0x3000);s.store(0x1000b400,4,0x105);
        rejected=false;try{s.pump_ipu_input();}catch(const hg::Fault&){rejected=true;}
        CHECK(rejected && c.tadr==0x3000 && c.qwc==0 && s.ipu.input_count==0);
    }
    // Normal SPR bursts: fixed directions, 16KB wrap, completion, and gating.
    {
        auto spr_storage=std::make_unique<hg::State>();auto& spr=*spr_storage;
        for(unsigned n=0;n<32;++n)spr.scratch[(0x3ff0+n)&0x3fff]=std::uint8_t(n+1);
        spr.store(0x1000d010,4,0x4000);spr.store(0x1000d080,4,0x3ff0);
        spr.store(0x1000d020,4,2);spr.store(0x1000d000,4,0x100);
        spr.dmac.held=true;CHECK(!spr.pump_scratchpad(8) && spr.ram[0x4000]==0);
        spr.dmac.held=false;CHECK(spr.pump_scratchpad(8));
        for(unsigned n=0;n<32;++n)CHECK(spr.ram[0x4000+n]==n+1);
        CHECK(spr.dmac.channels[8].madr==0x4020 && spr.dmac.channels[8].sadr==0x10);
        CHECK(spr.dmac.channels[8].qwc==0 && !(spr.dmac.channels[8].chcr&0x100));
        CHECK((spr.dmac.status&0x100) && !spr.pump_scratchpad(8));
        spr.store(0x1000d410,4,0x4000);spr.store(0x1000d480,4,0x100);
        spr.store(0x1000d420,4,2);spr.store(0x1000d400,4,0x100);
        CHECK(spr.pump_scratchpad(9));
        for(unsigned n=0;n<32;++n)CHECK(spr.scratch[0x100+n]==n+1);
        CHECK(spr.dmac.channels[9].sadr==0x120 && (spr.dmac.status&0x200));
        spr.store(0x1000d010,4,std::uint32_t(spr.ram.size()-16));
        spr.store(0x1000d020,4,2);spr.store(0x1000d000,4,0x100);
        bool rejected=false;try {spr.pump_scratchpad(8);}catch(const hg::Fault&){rejected=true;}
        CHECK(rejected && spr.dmac.channels[8].qwc==2 && (spr.dmac.channels[8].chcr&0x100));
        CHECK(spr.ram.back()==0);
        rejected=false;try {spr.store(0x1000d400,4,0x108);}catch(const hg::Fault&){rejected=true;}
        CHECK(rejected && !(spr.dmac.channels[9].chcr&0x100));
    }
    {
        auto storage=std::make_unique<hg::State>();auto& s=*storage;auto& c=s.dmac.channels[9];
        s.store(0x3000,8,0x0000400030000001ull); // REF
        s.store(0x3010,8,0x0000401000000001ull); // REFE
        for(unsigned n=0;n<32;++n)s.ram[0x4000+n]=std::uint8_t(n+1);
        s.store(0x1000d430,4,0x3000);s.store(0x1000d480,4,0x3ff0);s.store(0x1000d400,4,0x105);
        CHECK(s.pump_scratchpad(9) && c.tadr==0x3010 && c.sadr==0 && (c.chcr&0x100) && !(s.dmac.status&0x200));
        CHECK(s.pump_scratchpad(9) && c.tadr==0x3020 && c.sadr==16 && !(c.chcr&0x100) && (s.dmac.status&0x200));
        for(unsigned n=0;n<32;++n)CHECK(s.scratch[(0x3ff0+n)&0x3fff]==n+1);
        s.dmac.status=0;s.store(0x3000,8,0x00004000b0000001ull); // IRQ REF + TIE
        s.store(0x1000d430,4,0x3000);s.store(0x1000d400,4,0x185);
        CHECK(s.pump_scratchpad(9) && !(c.chcr&0x100) && (s.dmac.status&0x200));
        s.store(0x3000,8,0x0000000010000001ull);s.store(0x3010,8,0x8877665544332211ull); // CNT inline
        s.store(0x3020,8,0x0000304020000000ull);s.store(0x3040,8,0x0000000070000000ull); // NEXT, END
        s.store(0x1000d430,4,0x3000);s.store(0x1000d480,4,0x100);s.store(0x1000d400,4,0x105);
        CHECK(s.pump_scratchpad(9) && c.tadr==0x3020 && s.scratch[0x100]==0x11);
        CHECK(s.pump_scratchpad(9) && c.tadr==0x3040 && (c.chcr&0x100));
        CHECK(s.pump_scratchpad(9) && !(c.chcr&0x100));
        s.store(0x3000,8,0x0200000030000001ull);s.store(0x1000d430,4,0x3000);s.store(0x1000d400,4,0x105);
        bool fault=false;try{s.pump_scratchpad(9);}catch(const hg::Fault&){fault=true;}CHECK(fault && c.tadr==0x3000 && c.qwc==0);
    }
    auto s_storage=std::make_unique<hg::State>();auto& s=*s_storage;
    CHECK(s.load(0x1000e010,4,false)==0);
    s.dmac.raise(0x2021);
    CHECK(!s.dmac.pending());
    s.store(0xb000e010,4,0x00200001);
    CHECK(s.load(0x1000e010,4,false)==0x00202020 && s.dmac.pending());
    s.store(0x1000e010,4,0x00200020);
    CHECK(s.load(0x1000e010,4,false)==0x2000 && !s.dmac.pending());
    s.dmac.raise(0x8000);CHECK(s.dmac.pending());
    s.store(0x1000e010,4,0xffffffff);
    CHECK(s.load(0x1000e010,4,false)==0x63ff0000 && !s.dmac.pending());
    s.store(0x1000c010,4,0x1234567f);CHECK(s.load(0xb000c010,4,false)==0x12345670);
    s.store(0x1000d010,4,0xffffffff);CHECK(s.load(0x1000d010,4,false)==0x7ffffff0);
    s.store(0x1000d080,4,0xffffffff);CHECK(s.load(0x1000d080,4,false)==0x3ff0);
    s.store(0x1000c020,4,0x12340042);CHECK(s.load(0x1000c020,4,false)==0x42);
    // Normal GIF DMA feeds an EOP packet to the bounded path and raises D_STAT.
    s.store(0x6000,8,0x1000000000008001ull);s.store(0x6008,8,0xe);
    s.store(0x6010,8,0x1234);s.store(0x6018,8,0x3d);
    s.store(0x1000a010,4,0x6000);s.store(0x1000a020,4,2);s.store(0x1000a000,4,0x100);
    CHECK(s.pump_gif());CHECK(s.pump_gif());
    CHECK(s.gs.written[0x3d] && s.gs.value[0x3d]==0x1234 && !(s.dmac.channels[2].chcr&0x100) && (s.dmac.status&4));
    // VIF1 normal DMA carries PATH2 DIRECT/DIRECTHL qwords into the same GIF state.
    auto vif_storage=std::make_unique<hg::State>();auto& vif=*vif_storage;
    vif.store(0x7000,8,0);vif.store(0x7008,8,std::uint64_t(0x50000001u)<<32);
    vif.store(0x7010,8,0x1000000000008001ull);vif.store(0x7018,8,0xe);
    vif.store(0x7020,8,0);vif.store(0x7028,8,std::uint64_t(0x51000001u)<<32);
    vif.store(0x7030,8,0x4321);vif.store(0x7038,8,0x3d);
    vif.store(0x10009010,4,0x7000);vif.store(0x10009020,4,4);vif.store(0x10009000,4,0x100);
    while(vif.pump_vif1()){}
    CHECK(vif.gs.written[0x3d] && vif.gs.value[0x3d]==0x4321 && !(vif.dmac.channels[1].chcr&0x100) && (vif.dmac.status&2));
    // VIF1 source-chain END tag uses the same VIF/GIF endpoint and completion bit.
    auto vif_chain_storage=std::make_unique<hg::State>();auto& vif_chain=*vif_chain_storage;
    vif_chain.store(0x7100,8,0x70000004);vif_chain.store(0x7108,8,0);
    vif_chain.store(0x7110,8,0);vif_chain.store(0x7118,8,std::uint64_t(0x50000001u)<<32);
    vif_chain.store(0x7120,8,0x1000000000008001ull);vif_chain.store(0x7128,8,0xe);
    vif_chain.store(0x7130,8,0);vif_chain.store(0x7138,8,std::uint64_t(0x51000001u)<<32);
    vif_chain.store(0x7140,8,0x6789);vif_chain.store(0x7148,8,0x3d);
    vif_chain.store(0x10009030,4,0x7100);vif_chain.store(0x10009000,4,0x104);
    while(vif_chain.pump_vif1()){}
    CHECK(vif_chain.gs.written[0x3d] && vif_chain.gs.value[0x3d]==0x6789 && !(vif_chain.dmac.channels[1].chcr&0x100) && (vif_chain.dmac.status&2));
    // TTE delivers the upper tag words before payload, preserving DIRECT alignment.
    auto vif_tag_storage=std::make_unique<hg::State>();auto& vif_tag=*vif_tag_storage;
    vif_tag.store(0x7200,8,0x70000002);
    vif_tag.store(0x7208,8,std::uint64_t(0x50000002u)<<32);
    vif_tag.store(0x7210,8,0x1000000000008001ull);vif_tag.store(0x7218,8,0xe);
    vif_tag.store(0x7220,8,0x9876);vif_tag.store(0x7228,8,0x3d);
    vif_tag.store(0x10009030,4,0x7200);vif_tag.store(0x10009000,4,0x145);
    while(vif_tag.pump_vif1()){}
    CHECK(vif_tag.gs.written[0x3d] && vif_tag.gs.value[0x3d]==0x9876);
    CHECK(vif_tag.vif1.pending.empty() && !(vif_tag.dmac.channels[1].chcr&0x100));
    CHECK(vif_tag.dmac.status&2);
    // EE User's Manual pp45-46,74,77: VIF1 CALL pushes the inline
    // continuation and RET pops it. Two nested calls use ASR0 then ASR1.
    auto vif_stack_storage=std::make_unique<hg::State>();auto& vif_stack=*vif_stack_storage;
    vif_stack.store(0x7300,8,(std::uint64_t(0x7400)<<32)|0x50000000u);
    vif_stack.store(0x7310,8,0x70000000u);
    vif_stack.store(0x7400,8,(std::uint64_t(0x7500)<<32)|0x50000000u);
    vif_stack.store(0x7410,8,0x60000000u);
    vif_stack.store(0x7500,8,0x60000000u);
    vif_stack.store(0x10009030,4,0x7300);vif_stack.store(0x10009000,4,0x104);
    CHECK(vif_stack.pump_vif1());
    CHECK(vif_stack.dmac.channels[1].tadr==0x7400 && vif_stack.dmac.channels[1].asr0==0x7310 && ((vif_stack.dmac.channels[1].chcr>>4)&3)==1);
    CHECK(vif_stack.pump_vif1());
    CHECK(vif_stack.dmac.channels[1].tadr==0x7500 && vif_stack.dmac.channels[1].asr1==0x7410 && ((vif_stack.dmac.channels[1].chcr>>4)&3)==2);
    CHECK(vif_stack.pump_vif1());
    CHECK(vif_stack.dmac.channels[1].tadr==0x7410 && ((vif_stack.dmac.channels[1].chcr>>4)&3)==1);
    CHECK(vif_stack.pump_vif1());
    CHECK(vif_stack.dmac.channels[1].tadr==0x7310 && ((vif_stack.dmac.channels[1].chcr>>4)&3)==0);
    CHECK(vif_stack.pump_vif1());
    CHECK(!(vif_stack.dmac.channels[1].chcr&0x100) && (vif_stack.dmac.status&2));
    bool fault=false;
    try{s.store(0x1000c000,4,0x100);}catch(const hg::Fault&){fault=true;}
    CHECK(fault && s.load(0x1000c000,4,false)==0);
    fault=false;try{s.load(0x1000c030,4,false);}catch(const hg::Fault&){fault=true;}CHECK(fault);
    fault=false;try{s.load(0x1000e010,8,false);}catch(const hg::Fault&){fault=true;}CHECK(fault);
    // Actual packet input: cnt continues, end completes; tag bytes aren't stored.
    s.store(0x1000c020,4,0);s.store(0x1000c000,4,0x184);
    unsigned writes=0;
    auto write=[&](std::uint32_t a,std::uint64_t lo,std::uint64_t hi){
        s.memory(a,8);s.memory(a+8,8);s.store(a,8,lo);s.store(a+8,8,hi);++writes;
    };
    s.dmac.receive_sif0((0x2000ull<<32)|0x10000002,0,write);
    CHECK(writes==0 && s.dmac.channels[5].qwc==2);
    s.dmac.receive_sif0(0x1234,0x5678,write);
    CHECK(s.load(0x2000,8,false)==0x1234 && s.dmac.channels[5].qwc==1);
    s.dmac.receive_sif0(0x9abc,0xdef0,write);
    CHECK(s.dmac.channels[5].chcr&0x100);
    s.dmac.receive_sif0((0x3000ull<<32)|0x70000001,0,write);
    s.dmac.receive_sif0(0x1111,0x2222,write);
    CHECK(writes==3 && !(s.dmac.channels[5].chcr&0x100) && (s.dmac.status&0x20));
    CHECK(s.load(0x3008,8,false)==0x2222 && s.dmac.channels[5].madr==0x3010);
    s.store(0x1000e010,4,0x20);s.store(0x1000c000,4,0x184);
    // Zero-length IRQ cnt stops at the tag boundary, with no payload write.
    s.dmac.receive_sif0(0x90000000,0,write);
    CHECK(writes==3 && !(s.dmac.channels[5].chcr&0x100) && (s.dmac.status&0x20));
    s.store(0x1000c000,4,0x184);
    s.dmac.receive_sif0((0x2000000ull<<32)|0x70000001,0,write);
    fault=false;try{s.dmac.receive_sif0(1,2,write);}catch(const hg::Fault&){fault=true;}
    CHECK(fault && s.dmac.channels[5].qwc==1 && (s.dmac.channels[5].chcr&0x100));
    auto destination_storage=std::make_unique<hg::State>();auto& destination=*destination_storage;
    destination.store(0x1000c000,4,0x184);
    destination.receive_sif0((0x80000020ull<<32)|0x70000001,0);
    destination.receive_sif0(0x1122334455667788ull,0x8877665544332211ull);
    CHECK(destination.load(0x70000020,8,false)==0x1122334455667788ull);
    CHECK(destination.load(0x70000028,8,false)==0x8877665544332211ull);
    // Entire source-chain path, with a one-qword FIFO forcing backpressure.
    hg::IopState iop;
    iop.sif=std::make_shared<hg::SifLink>(1);
    // LIBSD polls and then starts the original SPU DMA channel. The bounded
    // IOP-to-SPU profile copies the complete descriptor synchronously.
    CHECK(iop.load(0x1f8010c8,4,false)==0);
    for(unsigned n=0;n<32;++n)iop.store(0x1200+n,1,n);
    iop.store(0x1f9001a8,2,0);iop.store(0x1f9001aa,2,8);
    iop.store(0x1f8010c0,4,0x1200);iop.store(0x1f8010c4,4,0x00020004);
    iop.store(0x1f8010c8,4,0x01000201);
    CHECK(iop.spu2_dma_count==1 && iop.spu2_dma_bytes==32 && iop.last_spu2_dma_destination==16);
    CHECK(iop.load(0x1f8010c0,4,false)==0x1220 && iop.load(0x1f8010c4,4,false)==4);
    CHECK(iop.load(0x1f8010c8,4,false)==0x201 && (iop.dmac.completed_channels&(1u<<4)));
    for(unsigned n=0;n<32;++n)CHECK(iop.spu2_ram[16+n]==n);
    fault=false;try{iop.store(0x1f8010c8,4,0x01000200);}catch(const hg::IopFault&){fault=true;}
    CHECK(fault && iop.spu2_dma_count==1);
    iop.dmac.completed_channels=0;
    auto ee_storage=std::make_unique<hg::State>();auto& ee=*ee_storage;
    ee.store(0x1000c000,4,0x184);
    ee.store(0x3fff,1,0x5a);ee.store(0x4020,1,0xa5);
    iop.store(0x1000,4,0xc0002000);iop.store(0x1004,4,8);
    iop.store(0x1008,4,0x90000002);iop.store(0x100c,4,0x4000);
    for(unsigned n=0;n<8;++n)iop.store(0x2000+n*4,4,0x12340000+n);
    iop.store(0x1f80152c,4,0x1000);iop.store(0x1f801528,4,0x01000701);
    CHECK(iop.pump_sif0() && iop.sif->pending_main()==1);
    CHECK(!iop.pump_sif0() && iop.dmac.channel[1][0]==0x2000);
    CHECK(iop.dmac.source_words==8 && iop.dmac.completed_channels==0);
    fault=false;try{iop.store(0x1f80152c,4,0x2000);}catch(const hg::IopFault&){fault=true;}
    CHECK(fault && iop.dmac.channel[1][3]==0x1010);
    auto receive=[&] {
        auto q=*iop.sif->peek_main();ee.receive_sif0(q.low,q.high);iop.sif->consume_main();
    };
    receive();CHECK(ee.dmac.channels[5].qwc==2);
    CHECK(iop.pump_sif0());receive();CHECK(ee.dmac.channels[5].qwc==1);
    CHECK(iop.pump_sif0());receive();
    for(unsigned n=0;n<8;++n)CHECK(ee.load(0x4000+n*4,4,false)==0x12340000+n);
    CHECK(ee.load(0x3fff,1,false)==0x5a && ee.load(0x4020,1,false)==0xa5);
    CHECK(!(ee.dmac.channels[5].chcr&0x100) && (ee.dmac.status&0x20));
    CHECK(!(iop.dmac.channel[1][2]&0x01000000) && (iop.dmac.completed_channels&(1u<<9)));
    CHECK(!iop.sif->pending_main() && !iop.pump_sif0());
    CHECK(iop.sif->main_flags==0 && iop.sif->sub_flags==0);
    // History is bounded observability, not a transport-size restriction.
    hg::IopState large;large.sif=std::make_shared<hg::SifLink>(8);
    large.store(0x1100,4,0x80003000);large.store(0x1104,4,20);
    large.store(0x1108,4,0x10000005);large.store(0x110c,4,0x5000);
    for(unsigned n=0;n<20;++n)large.store(0x3000+n*4,4,0xabc00000+n);
    large.store(0x1f80152c,4,0x1100);large.store(0x1f801528,4,0x01000701);
    for(unsigned n=0;n<6;++n)CHECK(large.pump_sif0());
    const auto& large_record=large.dmac.sif0_history[0];
    CHECK(large_record.payload_words==16 && large_record.total_words==20 && large_record.truncated);
    CHECK(large_record.payload[15]==0xabc0000f && !(large.dmac.channel[1][2]&0x01000000));
    CHECK(large.load(0xbf801560,4,false)==0);
    large.dmac.completed_channels|=1u<<9;
    CHECK(large.load(0xbf801574,4,false)==0x04000000);
    large.store(0xbf801574,4,0x04012345);
    CHECK(large.load(0xbf801574,4,false)==0x00012345 && !(large.dmac.completed_channels&(1u<<9)));
    // A seven-word result emits two payload qwords and retains the final unused
    // lane, matching the bounded partial-qword policy used by shorter RPCs.
    iop.store(0x1004,4,7);iop.store(0x1f80152c,4,0x1000);
    iop.store(0x1f801528,4,0x01000701);
    ee.store(0x1000c000,4,0x184);
    CHECK(iop.pump_sif0());receive();CHECK(iop.pump_sif0());receive();CHECK(iop.pump_sif0());receive();
    CHECK(!(iop.dmac.channel[1][2]&0x01000000) && ee.load(0x401c,4,false)==0x12340003);
    // A disarmed receiver retains queued data; connecting the wrong bus fails.
    auto waiting_storage=std::make_unique<hg::State>();auto& waiting=*waiting_storage;hg::IopState producer;producer.sif=waiting.sif;
    CHECK(producer.sif->push_main({0x90000000,0}));
    CHECK(!hg::service_sif(waiting,producer) && producer.sif->pending_main()==1);
    waiting.store(0x1000c000,4,0x184);
    waiting.store(0x1000e000,4,0);
    CHECK(!hg::service_sif(waiting,producer) && producer.sif->pending_main()==1);
    waiting.store(0x1000e000,4,1);waiting.store(0x1000e020,4,0x80000000);
    CHECK(!hg::service_sif(waiting,producer) && producer.sif->pending_main()==1);
    waiting.store(0x1000e020,4,0x80200000);
    CHECK(hg::service_sif(waiting,producer) && !producer.sif->pending_main());
    fault=false;try{waiting.store(0x1000e000,4,5);}catch(const hg::Fault&){fault=true;}
    CHECK(fault && waiting.load(0x1000e000,4,false)==1);
    // The documented hold handshake permits STR/QWC edits only while every
    // channel is suspended, then restores the prior D_ENABLER state.
    auto suspended_storage=std::make_unique<hg::State>();auto& suspended=*suspended_storage;
    suspended.dmac.channels[3].chcr=0x101;suspended.dmac.channels[3].qwc=7;
    fault=false;try{suspended.store(0x1000b020,4,0);}catch(const hg::Fault&){fault=true;}CHECK(fault);
    suspended.store(0x1000f590,4,suspended.load(0x1000f520,4,false)|0x10000);
    CHECK(suspended.load(0x1000f520,4,false)==0x10000 && !suspended.dmac.enabled(3));
    suspended.store(0x1000b000,4,1);suspended.store(0x1000b020,4,0);
    suspended.store(0x1000f590,4,0);
    CHECK(suspended.load(0x1000f520,4,false)==0 && suspended.dmac.enabled(3));
    CHECK(suspended.dmac.channels[3].chcr==1 && suspended.dmac.channels[3].qwc==0);
    waiting.store(0x10008080,4,0);
    fault=false;try{waiting.store(0x10008080,4,1);}catch(const hg::Fault&){fault=true;}CHECK(fault);
    producer.sif=std::make_shared<hg::SifLink>();fault=false;
    try{hg::service_sif(waiting,producer);}catch(const std::runtime_error&){fault=true;}CHECK(fault);
    // Reverse path: EE refe source tag, IOP destination header, two data qwords.
    auto main_storage=std::make_unique<hg::State>();auto& main=*main_storage;hg::IopState sub;main.sif=std::make_shared<hg::SifLink>(1);sub.sif=main.sif;
    main.store(0x1000,8,(0x2000ull<<32)|3);main.store(0x1008,8,0);
    main.store(0x2000,8,(8ull<<32)|0xc0004000);main.store(0x2008,8,0);
    for(unsigned n=0;n<8;++n)main.store(0x2010+n*4,4,0xabcd0000+n);
    sub.store(0x3fff,1,0x5a);sub.store(0x4020,1,0xa5);
    main.store(0x1000c430,4,0x1000);main.store(0x1000c400,4,0x184);
    CHECK(hg::service_sif(main,sub));
    CHECK(!hg::service_sif(main,sub) && main.dmac.channels[6].qwc==2);
    CHECK(main.sif->pending_sub()==1 && sub.load(0x4000,4,false)==0);
    sub.store(0x1f801538,4,0x41000300);
    for(unsigned n=0;n<4;++n)hg::service_sif(main,sub);
    for(unsigned n=0;n<8;++n)CHECK(sub.load(0x4000+n*4,4,false)==0xabcd0000+n);
    CHECK(sub.load(0x3fff,1,false)==0x5a && sub.load(0x4020,1,false)==0xa5);
    CHECK(main.dmac.channels[6].qwc==0 && !(main.dmac.channels[6].chcr&0x100));
    CHECK(main.dmac.status&0x40);
    CHECK(sub.dmac.channel[2][2]==0x40000300 && (sub.dmac.completed_channels&(1u<<10)));
    CHECK(!main.sif->pending_sub() && !hg::service_sif(main,sub));
    fault=false;try{hg::initialize_sif_transport(main,sub);}catch(const std::runtime_error&){fault=true;}
    CHECK(fault && main.sif->main_flags==0);
    main.store(0x1000c000,4,0x184);hg::initialize_sif_transport(main,sub);
    CHECK(sub.load(0x1d000020,4,false)==0x10000 && main.sif->sub_flags==0);
    sub.store(0x1d000030,4,0x10000);sub.store(0x1d000030,4,0x20000);
    CHECK(main.sif->sub_flags==0x30000);
    // Observed six-word reply: no reads past source end; final high lanes retain
    // the preceding payload qword. IOP tag arrays require only word alignment.
    auto partial_ee_storage=std::make_unique<hg::State>();auto& partial_ee=*partial_ee_storage;hg::IopState partial_iop;partial_iop.sif=partial_ee.sif;
    const auto source_end=std::uint32_t(partial_iop.ram.size());
    for(unsigned n=0;n<6;++n)partial_iop.store(source_end-24+n*4,4,0x55550000+n);
    partial_iop.store(0x100c,4,0x80000000|(source_end-24));partial_iop.store(0x1010,4,6);
    partial_iop.store(0x1014,4,0x90000002);partial_iop.store(0x1018,4,0x4000);
    partial_iop.store(0x1f80152c,4,0x100c);partial_iop.store(0x1f801528,4,0x01000701);
    partial_ee.store(0x1000c000,4,0x184);
    while(hg::service_sif(partial_ee,partial_iop)){}
    for(unsigned n=0;n<6;++n)CHECK(partial_ee.load(0x4000+n*4,4,false)==0x55550000+n);
    CHECK(partial_ee.load(0x4018,4,false)==0x55550002 && partial_ee.load(0x401c,4,false)==0x55550003);
    CHECK(partial_iop.dmac.channel[1][0]==source_end && partial_iop.dmac.channel[1][3]==0x101c);
    partial_iop.store(0x100c,4,0x80000000|(source_end-4));partial_iop.store(0x1010,4,1);
    partial_iop.store(0x1014,4,0x90000001);partial_iop.store(0x1018,4,0x5000);
    partial_iop.store(0x1f80152c,4,0x100c);partial_iop.store(0x1f801528,4,0x01000701);
    partial_ee.store(0x1000c000,4,0x184);
    while(hg::service_sif(partial_ee,partial_iop)){}
    CHECK(partial_ee.load(0x5000,4,false)==0x55550005 && partial_iop.dmac.channel[1][0]==source_end);
    CHECK(partial_ee.load(0x5004,4,false)==0x55550005 && partial_ee.load(0x5008,4,false)==0x55550002);
    // A real LOADFILE reply uses a two-word source tag before a later chain tag.
    auto pair_ee_storage=std::make_unique<hg::State>();auto& pair_ee=*pair_ee_storage;hg::IopState pair_iop;pair_iop.sif=pair_ee.sif;
    pair_iop.store(0x6000,4,0x11112222);pair_iop.store(0x6004,4,0x33334444);
    pair_iop.store(0x1100,4,0x6000);pair_iop.store(0x1104,4,2);
    pair_iop.store(0x1108,4,0x10000001);pair_iop.store(0x110c,4,0x7000);
    pair_iop.store(0x1f80152c,4,0x1100);pair_iop.store(0x1f801528,4,0x01000701);
    pair_ee.store(0x1000c000,4,0x184);
    CHECK(hg::service_sif(pair_ee,pair_iop));CHECK(hg::service_sif(pair_ee,pair_iop));
    CHECK(pair_ee.load(0x7000,4,false)==0x11112222 && pair_ee.load(0x7004,4,false)==0x33334444);
    CHECK(pair_iop.dmac.channel[1][3]==0x1110 && pair_iop.dmac.channel[1][0]==0x6008);
    // LOADFILE's shared two-word reply buffer is transport data, not a native
    // semaphore handoff. In particular, word 1 can be stale because the original
    // command handlers only write word 0 before returning this buffer to SIFRPC.
    // SIF transport must therefore never interpret payload word 1 as a semaphore.
    struct LoadfileReplyCase {
        std::uint32_t signature0,signature1,signature2,semaphore_id,payload_words;
        std::uint32_t expected_signals;
    };
    const LoadfileReplyCase loadfile_cases[] = {
        {0x80000006u,0xd557cu,0xd6d80u,1,2,0},
        {0,0xd557cu,0xd6d80u,1,2,0},
        {0x80000006u,0,0xd6d80u,1,2,0},
        {0x80000006u,0xd557cu,0,1,2,0},
        {0x80000006u,0xd557cu,0xd6d80u,0,2,0},
        {0x80000006u,0xd557cu,0xd6d80u,2,2,0},
        {0x80000006u,0xd557cu,0xd6d80u,0xffffffffu,2,0},
        {0x80000006u,0xd557cu,0xd6d80u,1,1,0},
        {0x80000006u,0xd557cu,0xd6d80u,1,3,0},
        {0x80000006u,0xd557cu,0xd6d80u,1,4,0},
    };
    for(const auto& reply:loadfile_cases) {
        hg::IopState loadfile_iop;auto loadfile_ee_storage=std::make_unique<hg::State>();auto& loadfile_ee=*loadfile_ee_storage;loadfile_iop.sif=loadfile_ee.sif;
        loadfile_iop.store(0x9000,4,0);loadfile_iop.store(0x9004,4,0);
        loadfile_iop.store(0x9008,4,0);loadfile_iop.store(0x900c,4,1);
        CHECK(loadfile_iop.create_semaphore(0x9000)==1);
        loadfile_iop.store(0xd6d38,4,reply.signature0);
        loadfile_iop.store(0xd6d3c,4,reply.signature1);
        loadfile_iop.store(0xd6d40,4,reply.signature2);
        loadfile_iop.store(0x7000,4,0x12345678);loadfile_iop.store(0x7004,4,reply.semaphore_id);
        loadfile_iop.store(0x7008,4,0xaabbccdd);loadfile_iop.store(0x700c,4,0x11223344);
        // End this IOP chain explicitly; do not fetch an uninitialized next tag.
        loadfile_iop.store(0x1200,4,0x80007000u);loadfile_iop.store(0x1204,4,reply.payload_words);
        loadfile_iop.store(0x1208,4,0x10000001);loadfile_iop.store(0x120c,4,0x7000);
        loadfile_iop.store(0x1f80152c,4,0x1200);loadfile_iop.store(0x1f801528,4,0x01000701);
        loadfile_ee.store(0x1000c000,4,0x184);
        // Transport progress is true for the header, before any reply payload.
        CHECK(hg::service_sif(loadfile_ee,loadfile_iop));
        CHECK(loadfile_iop.dmac.last_sif0_payload_words==0);
        CHECK(loadfile_iop.semaphores[0].count==0);
        // Malformed replies still make transport progress and reach EE memory.
        CHECK(hg::service_sif(loadfile_ee,loadfile_iop));
        CHECK(loadfile_iop.dmac.last_sif0_payload_words==reply.payload_words);
        CHECK(loadfile_iop.semaphores[0].count==reply.expected_signals);
        CHECK(loadfile_ee.load(0x7000,4,false)==0x12345678u);
        if(reply.payload_words>=2)CHECK(loadfile_ee.load(0x7004,4,false)==reply.semaphore_id);
        CHECK(!(loadfile_iop.dmac.channel[1][2]&0x01000000u));
        CHECK(!hg::service_sif(loadfile_ee,loadfile_iop));
        CHECK(loadfile_iop.semaphores[0].count==reply.expected_signals);
    }
    // A semaphore signal delivered to an already-blocked waiter transfers the
    // token to that waiter. The resumed worker continues after WaitSema, so the
    // semaphore count must remain zero rather than retaining a duplicate token.
    hg::IopState handed_off;
    handed_off.store(0x9200,4,0);handed_off.store(0x9204,4,0);
    handed_off.store(0x9208,4,0);handed_off.store(0x920c,4,1);
    CHECK(handed_off.create_semaphore(0x9200)==1);
    handed_off.threads.push_back({});handed_off.threads[0].ready=true;handed_off.current_thread=1;
    CHECK(!handed_off.wait_semaphore(1));
    CHECK(handed_off.semaphores[0].count==0 && handed_off.threads[0].wait_semaphore==1 && !handed_off.threads[0].ready);
    handed_off.current_thread=0;CHECK(handed_off.signal_semaphore(1)==0);
    CHECK(handed_off.semaphores[0].count==0 && handed_off.threads[0].wait_semaphore==0 && handed_off.threads[0].granted_semaphore==1 && handed_off.threads[0].ready);
    handed_off.current_thread=1;CHECK(handed_off.wait_semaphore(1));
    CHECK(handed_off.semaphores[0].count==0 && handed_off.threads[0].granted_semaphore==0);
    handed_off.current_thread=0;
    CHECK(handed_off.signal_semaphore(1)==0 && handed_off.semaphores[0].count==1);
    CHECK(handed_off.signal_semaphore(1)==std::uint32_t(-420) && handed_off.semaphores[0].count==1);
    // Rejected FIFO writes must not publish payload metadata or duplicate
    // diagnostic history when the same payload is retried.
    hg::IopState stalled;
    stalled.store(0x7100,4,0x11112222);stalled.store(0x7104,4,0x33334444);
    stalled.store(0x1300,4,0x80007100u);stalled.store(0x1304,4,2);
    stalled.store(0x1308,4,0x10000001);stalled.store(0x130c,4,0x7100);
    stalled.store(0x1f80152c,4,0x1300);stalled.store(0x1f801528,4,0x01000701);
    const auto stalled_read=[&](std::uint32_t address){return stalled.load(address,4,false);};
    const auto accept_quad=[](std::uint64_t,std::uint64_t){return true;};
    const auto reject_quad=[](std::uint64_t,std::uint64_t){return false;};
    CHECK(stalled.dmac.pump_sif0(stalled_read,accept_quad));
    const auto& stalled_record=stalled.dmac.sif0_history[stalled.dmac.sif0_active_record];
    for(unsigned attempt=0;attempt<2;++attempt) {
        CHECK(!stalled.dmac.pump_sif0(stalled_read,reject_quad));
        CHECK(stalled.dmac.last_sif0_payload_words==0);
        CHECK(stalled_record.total_words==0 && stalled_record.payload_words==0);
        CHECK(!stalled_record.truncated && stalled.dmac.channel[1][0]==0x7100);
    }
    CHECK(stalled.dmac.pump_sif0(stalled_read,accept_quad));
    CHECK(stalled.dmac.last_sif0_payload_words==2);
    CHECK(stalled_record.total_words==2 && stalled_record.payload_words==2);
    CHECK(stalled_record.payload[0]==0x11112222u && stalled_record.payload[1]==0x33334444u);
    CHECK(stalled.dmac.channel[1][0]==0x7108);
    CHECK(!stalled.dmac.pump_sif0(stalled_read,accept_quad));
    // Diagnostic SIF1 history is bounded but must never change a valid
    // original DMA transfer merely because its packet exceeds the prefix.
    hg::IopState captured;
    captured.dmac.channel[2][2]=0x41000300;
    auto capture_write=[&](std::uint32_t address,std::uint64_t low,std::uint64_t high) {
        captured.store(address,4,std::uint32_t(low));captured.store(address+4,4,std::uint32_t(low>>32));
        captured.store(address+8,4,std::uint32_t(high));captured.store(address+12,4,std::uint32_t(high>>32));
    };
    captured.dmac.receive_sif1((68ull<<32)|0xc0004000u,0,capture_write);
    for(unsigned qword=0;qword<17;++qword)
        captured.dmac.receive_sif1(0xaaa00000u+qword,std::uint64_t(0xbbb00000u+qword)<<32,capture_write);
    const auto& record=captured.dmac.sif1_history[0];
    CHECK(record.count==68 && record.received_words==68 && record.captured_words==64 && record.truncated);
    CHECK(captured.load(0x4000,4,false)==0xaaa00000 && captured.load(0x4100,4,false)==0xaaa00010);
    CHECK(!(captured.dmac.channel[2][2]&0x01000000) && (captured.dmac.completed_channels&(1u<<10)));
}
