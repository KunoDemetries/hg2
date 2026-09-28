#include "hg/ee_interrupt.hpp"
#include "hg/display_timing.hpp"
#include <iostream>
#include <memory>
#define CHECK(x) do {if(!(x)){std::cerr<<"Failed line "<<__LINE__<<": "<<#x<<'\n';return 1;}}while(0)
int main() {
    hg::DisplayTiming clock;
    std::vector<bool> edges;
    auto edge=[&](bool start){edges.push_back(start);};
    clock.advance_us(1000000,edge);CHECK(edges.empty());
    clock.configure(true,1,2,0);
    clock.advance_us(15253,edge);CHECK(edges.empty());
    clock.advance_us(1,edge);CHECK(edges==std::vector<bool>{true});
    clock.advance_us(1429,edge);CHECK(edges.size()==1);
    clock.advance_us(1,edge);CHECK(edges==std::vector<bool>({true,false}) && clock.fields==1);
    // One second plus one millisecond is exactly 60 NTSC fields, with no drift.
    hg::DisplayTiming whole,split;whole.configure(true,1,2,0);split.configure(true,1,2,0);
    std::vector<bool> a,b;whole.advance_us(1001000,[&](bool x){a.push_back(x);});
    for(unsigned n=0;n<1001;++n)split.advance_us(1000,[&](bool x){b.push_back(x);});
    CHECK(a==b && a.size()==120 && whole.fields==60 && split.fields==60);
    bool bad=false;try{clock.configure(true,0,2,0);}catch(const std::runtime_error&){bad=true;}CHECK(bad);
    auto state=std::make_unique<hg::State>();auto& s=*state;s.boot.enabled=true;
    auto add=[&](unsigned pc,std::uint32_t next){
        s.w(3,0x10);s.w(4,3);s.w(5,pc);s.w(6,next);s.w(7,pc+1);hg::kernel_call(s);
        return unsigned(s.r(2));
    };
    CHECK(add(0x1000,0xffffffff)==1);CHECK(add(0x2000,0xffffffff)==2);
    CHECK(add(0x3000,2)==3);CHECK(add(0x4000,0)==4);
    bad=false;try{add(0x5000,99);}catch(const hg::Fault&){bad=true;}
    CHECK(bad && s.boot.interrupt_handlers.size()==4);
    s.pc=0x8000;s.last_transfer_pc=0x7000;s.w(28,0x9000);s.w(29,0xa000);
    s.gpr[8]={0x123456789abcdef0ull,0xfedcba9876543210ull};s.fpr[3]=0x3f800000;
    const auto before=s.save_cpu();
    hg::EeIntcInterruptDispatcher irq;std::vector<unsigned> calls;
    auto run=[&](hg::State& t,std::uint64_t){
        if(!t.boot.in_interrupt || t.r(4)!=3 || t.r(28)!=0x9000)throw std::runtime_error("bad callback context");
        calls.push_back(t.pc);t.gpr[8]={1,2};t.fpr[3]=0;t.w(2,0);t.pc=std::uint32_t(t.r(31));
    };
    s.intc.raise(3);CHECK(!irq.service(s,run));CHECK(s.intc.status==8);
    s.intc.mask=8;s.cp0_status&=~1u;CHECK(!irq.service(s,run));s.cp0_status=before.cp0_status;
    CHECK(irq.service(s,run) && irq.active() && calls==std::vector<unsigned>{0x4000});
    // A new edge during the chain remains pending after context restoration.
    s.intc.raise(3);
    while(irq.active())irq.service(s,run);
    CHECK(calls==std::vector<unsigned>({0x4000,0x1000,0x3000,0x2000}));
    CHECK(s.pc==before.pc && s.r(29)==0xa000 && s.last_transfer_pc==0x7000 && !s.boot.in_interrupt);
    CHECK(s.gpr[8].lo==before.gpr[8].lo && s.gpr[8].hi==before.gpr[8].hi && s.fpr[3]==before.fpr[3]);
    CHECK(s.intc.status==8 && s.cp0_status==before.cp0_status);
    calls.clear();irq.service(s,run,4);CHECK(!irq.active() && calls.size()==4 && s.intc.status==0);
    // Removed IDs retain their positions for already-registered relative insertions.
    s.w(3,0x11);s.w(4,3);s.w(5,2);hg::kernel_call(s);s.intc.raise(3);calls.clear();
    irq.service(s,run,4);CHECK(calls==std::vector<unsigned>({0x4000,0x1000,0x3000}));
    s.intc.raise(3);bad=false;
    try{irq.service(s,[](hg::State& t,std::uint64_t){t.w(2,1);t.pc=std::uint32_t(t.r(31));});}
    catch(const hg::Fault&){bad=true;}CHECK(bad);
    return 0;
}
