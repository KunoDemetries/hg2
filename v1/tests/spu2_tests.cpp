#include "hg/iop.hpp"
#include <iostream>
#include <vector>
#define CHECK(x) do {if(!(x)){std::cerr<<"check failed at "<<__LINE__<<": "<<#x<<'\n';return 1;}}while(0)
int main() {
    hg::IopState s;
    // Voice DMA completion is per core, follows committed bytes, and is
    // retired when LIBSD clears ATTR's transfer mode.
    for(unsigned core=0;core<2;++core) {
        const auto attr=0x1f90019au+core*0x400u,status=0x1f900344u+core*0x400u;
        const auto dma=core?0x1f801500u:0x1f8010c0u;
        CHECK(s.load(status,2,false)==0);
        s.store(attr,2,0x20);s.store(0x10000,4,0x12345678u+core);
        s.store(dma,4,0x10000);s.store(dma+4,4,0x00010010);s.store(dma+8,4,0x01000201);
        CHECK(s.load(status,2,false)==0x80 && s.spu2_ram[0]==0x78u+core);
        CHECK(s.load(0x1f900344u+(1-core)*0x400u,2,false)==0);
        s.store(attr,2,0);CHECK(s.load(status,2,false)==0);
        bool rejected=false;try{s.store(status,2,0x80);}catch(const hg::IopFault&){rejected=true;}
        CHECK(rejected && s.load(status,2,false)==0);
    }
    s.spu2_dma_count=0;s.spu2_dma_bytes=0;s.dmac.completed_channels=0;
    std::fill(s.spu2_ram.begin(),s.spu2_ram.end(),0);

    // Synthetic alternating blocks: each 256-sample L block is followed by R.
    for(unsigned half=0;half<4;++half)for(unsigned channel=0;channel<2;++channel)
        for(unsigned sample=0;sample<256;++sample)
            s.store(0x10000+half*1024+channel*512+sample*2,2,0x1000+channel*0x4000+half*256+sample);
    s.store(0x1f9001b0,2,1);
    s.store(0x1f8010c0,4,0x10000);s.store(0x1f8010c4,4,0x00200010);
    s.store(0x1f8010c8,4,0x01000201);
    CHECK(s.spu2_input.cores[0].ready[0] && s.spu2_input.cores[0].ready[1]);
    CHECK(s.spu2_dma_count==1 && s.spu2_dma_bytes==2048);
    CHECK(s.dmac.read(0x1f8010c0,4)==0x10800 && s.dmac.read(0x1f8010c4,4)==16);
    CHECK(s.dmac.completed_channels==(1u<<4));
    auto sample=[&](unsigned core,unsigned channel,unsigned frame) {
        const auto a=hg::Spu2Input::sample_address(core,channel,frame);
        return unsigned(s.spu2_ram[a]|(unsigned(s.spu2_ram[a+1])<<8));
    };
    CHECK(sample(0,0,0)==0x1000 && sample(0,1,0)==0x5000);
    CHECK(sample(0,0,511)==0x11ff && sample(0,1,511)==0x51ff);
    CHECK(s.spu2_ram[0]==0 && sample(1,0,0)==0);
    // A second descriptor must wait for actual consumed halves. It cannot
    // complete in a reentrant IRQ/rearm loop while both buffers are occupied.
    s.dmac.completed_channels=0;s.store(0x1f8010c4,4,0x00200010);
    s.store(0x1f8010c8,4,0x01000201);
    CHECK(s.spu2_input_dma[0].active && s.spu2_dma_count==1 && s.dmac.completed_channels==0);
    std::vector<std::array<unsigned,2>> frames;
    auto emit=[&](unsigned core,std::uint16_t left,std::uint16_t right) {
        if(core!=0)throw std::runtime_error("unexpected core");frames.push_back({left,right});
    };
    s.advance_spu2(5333,emit);
    CHECK(frames.size()==255 && s.dmac.read(0x1f8010c0,4)==0x10800);
    s.advance_spu2(1,emit);
    CHECK(frames.size()==256 && frames.back()[0]==0x10ff && frames.back()[1]==0x50ff);
    CHECK(s.dmac.read(0x1f8010c0,4)==0x10c00 && s.dmac.read(0x1f8010c4,4)==0x00100010);
    CHECK(s.dmac.completed_channels==0 && sample(0,0,0)==0x1200 && sample(0,1,0)==0x5200);
    s.advance_spu2(5333,emit);
    CHECK(frames.size()==512 && frames.back()[0]==0x11ff && frames.back()[1]==0x51ff);
    CHECK(!s.spu2_input_dma[0].active && s.spu2_dma_count==2 && s.spu2_dma_bytes==4096);
    CHECK(s.dmac.read(0x1f8010c0,4)==0x11000 && s.dmac.read(0x1f8010c4,4)==16);
    CHECK(s.dmac.completed_channels==(1u<<4) && !(s.dmac.read(0x1f8010c8,4)&0x01000000));
    // Arbitrary caller chunking preserves the fractional 48 kHz sample clock.
    for(unsigned n=0;n<1000;++n)s.advance_spu2(1,emit);
    CHECK(frames.size()==560 && frames.back()[0]==0x122f);
    // Cancel a pending descriptor without a completion; then disable input.
    s.dmac.completed_channels=0;s.store(0x1f8010c4,4,0x00200010);s.store(0x1f8010c8,4,0x01000201);
    CHECK(s.spu2_input_dma[0].active);
    s.store(0x1f8010c8,4,0x201);s.store(0x1f9001b0,2,0);s.advance_spu2(20000,emit);
    CHECK(frames.size()==560 && s.dmac.completed_channels==0 && s.spu2_dma_count==2);
    // Core 1 uses its own two channel areas, DMA channel and completion bit.
    s.store(0x1f9005b0,2,2);s.store(0x1f801500,4,0x10000);s.store(0x1f801504,4,0x00200010);
    s.store(0x1f801508,4,0x01000201);s.advance_spu2(1000);
    CHECK(s.spu2_input_frames==48 && s.spu2_input_history[47].core==1);
    CHECK(s.spu2_input_history[47].left==0x102f && s.spu2_input_history[47].right==0x502f);
    CHECK(s.dmac.completed_channels==(1u<<7));
    bool fault=false;
    try{s.store(0x1f9005b0,2,4);}catch(const hg::IopFault&){fault=true;}
    CHECK(fault && s.load(0x1f9005b0,2,false)==2);
    // No silence or stale-data policy is invented for an unprovided next half.
    fault=false;try{s.advance_spu2(11000);}catch(const hg::IopFault&){fault=true;}
    CHECK(fault);
    // Pending source corruption must be checked before any host memory read.
    hg::IopState changed;
    changed.store(0x1f9001b0,2,1);changed.store(0x1f8010c0,4,0x10000);
    changed.store(0x1f8010c4,4,0x00300010);changed.store(0x1f8010c8,4,0x01000201);
    changed.store(0x1f8010c0,4,0xfffffff0);
    fault=false;try{changed.advance_spu2(5334);}catch(const hg::IopFault&){fault=true;}
    CHECK(fault && changed.spu2_dma_count==0 && changed.spu2_dma_bytes==2048);
    std::cout<<"SPU2 input tests passed\n";
}
