#include "hg/runtime.hpp"
#include "hg/system.hpp"
#include "hg/ee_interrupt.hpp"
#include "hg/ee_thread.hpp"
#include <iostream>
#define CHECK(x) do { if (!(x)) { std::cerr << "Failed line " << __LINE__ << ": " << #x << '\n'; return 1; } } while (0)
int main() {
    for(unsigned cursor=0;cursor<5;++cursor)for(unsigned mask=0;mask<32;++mask) {
        hg::State scheduled;scheduled.boot.threads.resize(5);scheduled.boot.thread_cursor=cursor;
        unsigned expected=0,priority=128;
        for(unsigned n=0;n<5;++n) {
            auto& t=scheduled.boot.threads[n];t.status=(mask&(1u<<n))?2:0;
            t.priority=10+n%3;t.context.pc=0x1000;t.initialized=true;
        }
        for(unsigned offset=0;offset<5;++offset) {
            const unsigned n=(cursor+offset)%5;const auto& t=scheduled.boot.threads[n];
            if(t.status==2 && t.priority<priority){expected=n+1;priority=t.priority;}
        }
        unsigned selected=0;
        const auto ran=hg::service_ee_thread(scheduled,[&](hg::State& s,std::uint64_t){selected=s.boot.current_thread;});
        CHECK(selected==expected && ran==(expected!=0));
    }
    hg::State s;
    s.w(3,0x3c); s.w(4,0x2000); s.w(5,~0ull); s.w(6,0x8000); s.w(7,0x1000); s.w(8,0x4000);
    s.boot.executable_name="synthetic.elf";
    bool fault=false;
    try {hg::kernel_call(s);} catch(const hg::Fault&) {fault=true;}
    CHECK(fault && !s.boot.thread_ready);
    s.boot.enabled=true;
    hg::kernel_call(s);
    CHECK(s.boot.thread_ready && s.r(2)==0x1ffed60 && s.boot.stack_base==0x1ff7000);
    CHECK(s.load(0x1000,4,false)==1 && s.load(0x1004,4,false)==0x1044 && s.load(0x1008,4,false)==0);
    CHECK(std::string(reinterpret_cast<const char*>(s.ram.data()+0x1044))=="synthetic.elf");
    s.w(3,0x3d); s.w(4,0x100000); s.w(5,~0ull); hg::kernel_call(s);
    CHECK(s.boot.heap_start==0x100000 && s.boot.heap_end==0x1ff7000);
    s.w(3,0x3e); hg::kernel_call(s); CHECK(s.r(2)==0x1ff7000);
    s.store(0x2004,4,3); s.store(0x2008,4,2);
    s.w(3,0x40); s.w(4,0x2000); hg::kernel_call(s);
    CHECK(s.r(2)==1 && s.boot.semaphores.size()==1 && s.boot.semaphores[0].count==2);
    s.store(0x2008,4,4); fault=false;
    try {hg::kernel_call(s);} catch(const hg::Fault&) {fault=true;}
    CHECK(fault && s.boot.semaphores.size()==1);
    s.w(3,0x45); fault=false;
    try {hg::kernel_call(s);} catch(const hg::Fault&) {fault=true;}
    CHECK(fault);
    s.w(4,1);hg::kernel_call(s);CHECK(s.r(2)==1 && s.boot.semaphores[0].count==1);
    hg::kernel_call(s);CHECK(s.r(2)==1 && s.boot.semaphores[0].count==0);
    const auto poll_thread=s.boot.current_thread,poll_status=s.boot.threads[0].status;
    hg::kernel_call(s);
    CHECK(s.r(2)==hg::sx32(0xffffffffu) && s.boot.semaphores[0].count==0);
    CHECK(s.boot.current_thread==poll_thread && s.boot.threads[0].status==poll_status);
    s.boot.semaphores[0].count=3;s.w(3,0x42);hg::kernel_call(s);
    CHECK(s.r(2)==hg::sx32(0xffffffffu) && s.boot.semaphores[0].count==3);
    s.boot.in_interrupt=true;s.w(3,hg::sx32(0xffffffbdu));hg::kernel_call(s);
    CHECK(s.r(2)==hg::sx32(0xffffffffu) && s.boot.semaphores[0].count==3);
    s.boot.in_interrupt=false;s.boot.semaphores[0].count=0;
    s.w(3,0x7a);s.w(4,0x80000000);hg::kernel_call(s);CHECK(s.r(2)==0);
    s.boot.sif_system_registers[0]=0x12000;
    s.w(4,0x80000000);hg::kernel_call(s);CHECK(s.r(2)==0x12000);
    s.sif->sub_flags=0x20000;s.w(4,4);hg::kernel_call(s);CHECK(s.r(2)==0x20000);
    s.w(4,3);hg::kernel_call(s);CHECK(s.r(2)==0);
    s.w(4,0x80000003);fault=false;
    try{hg::kernel_call(s);}catch(const hg::Fault&){fault=true;}CHECK(fault);
    hg::State bad; bad.boot.enabled=true; bad.w(3,0x3c); bad.w(5,~0ull);
    bad.w(6,0x8000); bad.w(7,0xfffffff0); bad.w(8,0x4000);
    fault=false;
    try {hg::kernel_call(bad);} catch(const hg::Fault&) {fault=true;}
    CHECK(fault && !bad.boot.thread_ready);
    s.w(3,0x74); s.w(4,0x83); s.w(5,0x4000); hg::kernel_call(s);
    CHECK(s.load(hg::BootServices::table+0x83*4,4,false)==0x4000);
    s.pc=0x2000; s.w(31,0x5000); s.w(3,0x83); hg::kernel_call(s);
    CHECK(s.pc==0x4000 && s.r(31)==hg::BootServices::return_gateway);
    s.pc=hg::BootServices::return_gateway;
    CHECK(hg::return_from_kernel(s) && s.pc==0x2004 && s.r(31)==0x5000);
    s.w(3,0x64); s.w(4,3); hg::kernel_call(s);
    CHECK(s.boot.data_cache_epoch==1 && s.boot.instruction_cache_epoch==1);
    s.w(3,0x10); s.w(4,11); s.w(5,0x8000); s.w(6,0); hg::kernel_call(s);
    CHECK(s.r(2)==1 && s.boot.interrupt_handlers[0].callback==0x8000);
    s.w(3,0x14); s.w(4,11); hg::kernel_call(s);
    CHECK(s.r(2)==1 && s.intc.mask==1u<<11);
    fault=false; try {hg::kernel_call(s);} catch(const hg::Fault&) {fault=true;}
    CHECK(fault && s.intc.mask==1u<<11);
    s.w(3,0x15);hg::kernel_call(s);CHECK(s.r(2)==1 && s.intc.mask==0);
    s.w(3,0x14);hg::kernel_call(s);CHECK(s.intc.mask==1u<<11);
    s.boot.in_interrupt=true;s.w(3,hg::sx32(0xffffffd1u));hg::kernel_call(s);
    CHECK(s.r(2)==1);s.boot.in_interrupt=false;
    s.w(3,0x11);s.w(4,11);s.w(5,1);hg::kernel_call(s);
    CHECK(s.r(2)==0 && !s.boot.interrupt_handlers[0].callback);
    s.store(0x3004,4,0x4000); s.store(0x3008,4,0x8000); s.store(0x300c,4,0x400);
    s.store(0x3010,4,0x5000); s.store(0x3014,4,12); s.store(0x301c,4,7);
    s.w(3,0x20); s.w(4,0x3000); hg::kernel_call(s);
    CHECK(s.r(2)==2 && s.boot.threads.size()==2 && s.boot.threads[1].status==0x10);
    CHECK(s.boot.threads[1].stack==0x8000 && s.boot.threads[1].priority==12 && s.boot.threads[1].attr==7);
    s.w(3,0x22);s.w(4,2);s.w(5,0x12345678);hg::kernel_call(s);
    CHECK(s.r(2)==0 && s.boot.threads[1].status==2 && s.boot.threads[1].initialized);
    CHECK(s.boot.threads[1].context.pc==0x4000 && s.boot.threads[1].context.gpr[4].lo==0x12345678);
    CHECK(s.boot.threads[1].context.gpr[28].lo==0x5000 && s.boot.threads[1].context.gpr[29].lo==0x83f0);
    const auto root_pc=s.pc;bool worker_ran=false,worker_context_ok=false;
    CHECK(hg::service_ee_thread(s,[&](hg::State& active,std::uint64_t){worker_context_ok=active.boot.current_thread==1;}));
    CHECK(worker_context_ok);
    s.boot.threads[0].status=0x10;s.boot.current_thread=0;
    CHECK(hg::service_ee_thread(s,[&](hg::State& worker,std::uint64_t count){
        worker_context_ok=count==1 && worker.pc==0x4000 && worker.boot.current_thread==2;
        worker.w(3,0x23);hg::kernel_call(worker);worker_ran=true;
    }));
    CHECK(worker_ran && worker_context_ok && s.pc==0xff003000 && s.boot.current_thread==0 && s.boot.threads[1].status==0x10);
    s.pc=root_pc;s.boot.current_thread=1;s.boot.threads[0].status=1;
    s.w(3,0x30);s.w(4,2);s.w(5,0x3800);hg::kernel_call(s);
    CHECK(s.r(2)==0 && s.load(0x3800,4,false)==0x10 && s.load(0x3804,4,false)==0x4000);
    CHECK(s.load(0x3808,4,false)==0x8000 && s.load(0x380c,4,false)==0x400);
    CHECK(s.load(0x3810,4,false)==0x5000 && s.load(0x3814,4,false)==12);
    CHECK(s.load(0x3818,4,false)==12 && s.load(0x381c,4,false)==7);
    s.boot.in_interrupt=true;s.w(3,hg::sx32(0xffffffcfu));s.w(5,0x3900);hg::kernel_call(s);
    CHECK(s.r(2)==0 && s.boot.current_thread==1 && s.boot.threads[1].status==0x10);
    for(unsigned n=0;n<0x30;n+=4)CHECK(s.load(0x3800+n,4,false)==s.load(0x3900+n,4,false));
    s.boot.in_interrupt=false;
    s.boot.threads[1].status=4;s.boot.threads[1].wait_type=1;s.w(3,0x33);s.w(4,2);hg::kernel_call(s);
    CHECK(s.r(2)==0 && s.boot.threads[1].status==2 && s.boot.threads[1].wait_type==0 && s.boot.threads[1].wakeup_count==0);
    hg::kernel_call(s);CHECK(s.boot.threads[1].wakeup_count==1);
    s.boot.threads[1].status=4;s.boot.threads[1].wait_type=1;s.boot.in_interrupt=true;
    s.w(3,hg::sx32(0xffffffccu));s.w(4,2);hg::kernel_call(s);
    CHECK(s.boot.threads[1].status==2);s.boot.in_interrupt=false;
    s.w(3,0x35);hg::kernel_call(s);CHECK(s.r(2)==1 && s.boot.threads[1].wakeup_count==0);
    s.w(3,0x37);hg::kernel_call(s);CHECK(s.r(2)==2 && s.boot.threads[1].status==8);
    s.w(3,0x39);hg::kernel_call(s);CHECK(s.r(2)==2 && s.boot.threads[1].status==2);
    s.boot.threads.push_back(s.boot.threads[1]);s.boot.threads[2].context.pc=0x4100;
    s.boot.thread_cursor=1;s.w(3,0x2b);s.w(4,12);hg::kernel_call(s);CHECK(s.boot.thread_cursor==2);
    s.boot.threads.pop_back();s.boot.thread_cursor=1;
    s.boot.threads[1].status=0x10;
    s.w(3,0x37);s.w(4,2);hg::kernel_call(s);
    CHECK(s.r(2)==hg::sx32(0xffffffffu) && s.boot.threads[1].status==0x10);
    s.w(3,0x7f);hg::kernel_call(s);CHECK(s.r(2)==32*1024*1024);
    s.store(0x3900,4,0x3910);s.store(0x3910,1,'o');s.store(0x3911,1,'k');s.store(0x3912,1,0);
    s.w(3,0x7c);s.w(4,0x10);s.w(5,0x3900);hg::kernel_call(s);
    CHECK(s.r(2)==1 && s.boot.deci2_output=="ok" && s.boot.deci2_kputs_calls==1);
    s.store(0x300c,4,0xffffffff);s.w(3,0x20);s.w(4,0x3000);fault=false;
    try {hg::kernel_call(s);} catch(const hg::Fault&) {fault=true;}
    CHECK(fault && s.boot.threads.size()==2);
    s.w(3,0x2f); hg::kernel_call(s); CHECK(s.r(2)==1);
    s.w(3,0x29); s.w(4,1); s.w(5,1); hg::kernel_call(s);
    CHECK(s.r(2)==0 && s.boot.threads[0].priority==1);
    s.w(3,0x29); s.w(4,0); s.w(5,5); hg::kernel_call(s);
    CHECK(s.r(2)==1 && s.boot.threads[0].priority==5);
    const auto saved_priority=s.r(2);
    s.w(3,0x29); s.w(4,1); s.w(5,saved_priority); hg::kernel_call(s);
    CHECK(s.r(2)==5 && s.boot.threads[0].priority==1);
    s.console.language=4; s.console.screen_type=2; s.console.timezone=60;
    s.w(3,0x02);s.w(4,1);s.w(5,2);s.w(6,1);hg::kernel_call(s);
    CHECK(s.gs.crtc_configured && s.gs.crtc_interlace==1 && s.gs.crtc_mode==2 && s.gs.crtc_frame==1);
    s.w(3,0x71);s.w(4,0x1122334455667788ull);hg::kernel_call(s);
    CHECK(s.gs.privileged_imr==0x1122334455667788ull);
    s.w(3,0x70);hg::kernel_call(s);CHECK(s.r(2)==0x1122334455667788ull);
    s.w(3,0x4b); s.w(4,0x4000); hg::kernel_call(s);
    CHECK(s.load(0x4000,4,false)==0x07842014);
    s.store(0x4000,4,0x00032019); s.w(3,0x4a); hg::kernel_call(s);
    CHECK(s.console.language==3 && s.console.component_output==1 && s.console.spdif_disabled==1);
    s.w(2,0xabcdef);s.w(3,0x78);hg::kernel_call(s);
    CHECK(s.dmac.channels[5].chcr==0x184 && s.dmac.status==0 && s.r(2)==0xabcdef);
    s.w(3,0x6b);hg::kernel_call(s);
    CHECK(s.dmac.channels[5].chcr==0x84 && s.dmac.status==0 && s.r(2)==0xabcdef);
    s.w(3,0x78);hg::kernel_call(s);
    s.dmac.channels[5].qwc=1;s.w(3,0x6b);fault=false;
    try{hg::kernel_call(s);}catch(const hg::Fault&){fault=true;}
    CHECK(fault && s.dmac.channels[5].chcr==0x184 && s.dmac.channels[5].qwc==1);
    s.dmac.channels[5].qwc=0;
    s.w(3,0x12);s.w(4,5);s.w(5,0x9000);s.w(6,0);s.w(7,0x1122);hg::kernel_call(s);
    CHECK(s.r(2)==1 && s.boot.dma_handlers.size()==1 && s.boot.dma_handlers[0].callback==0x9000);
    CHECK(s.boot.dma_handlers[0].arg==0x1122 && s.boot.interrupt_handlers.size()==1);
    s.w(3,0x16);s.w(4,5);hg::kernel_call(s);
    CHECK(s.r(2)==1 && s.dmac.mask==0x20 && s.dmac.status==0);
    fault=false;try{hg::kernel_call(s);}catch(const hg::Fault&){fault=true;}
    CHECK(fault && s.dmac.mask==0x20);
    s.w(3,0x17);s.w(4,5);hg::kernel_call(s);CHECK(s.r(2)==1 && s.dmac.mask==0);
    s.w(3,0x13);s.w(5,1);hg::kernel_call(s);CHECK(s.r(2)==0 && !s.boot.dma_handlers[0].callback);
    s.w(3,0x12);s.w(5,0x9000);hg::kernel_call(s);CHECK(s.r(2)==1 && s.boot.dma_handlers.size()==1);
    hg::State main;main.boot.enabled=true;main.w(3,0x79);main.w(4,0x80000002);main.w(5,1);
    hg::kernel_call(main);CHECK(main.r(2)==1 && main.boot.sif_system_registers[2]==1);
    main.sif->sub_flags=0x70000;main.w(4,4);main.w(5,0x40000);hg::kernel_call(main);
    CHECK(main.sif->sub_flags==0x30000);
    main.w(4,3);main.w(5,0x10000);hg::kernel_call(main);
    main.w(5,0x20000);hg::kernel_call(main);CHECK(main.sif->main_flags==0x30000);
    hg::IopState sub;sub.sif=main.sif;sub.store(0x1f801538,4,0x41000300);
    main.store(0x1000,4,0x2000);main.store(0x1004,4,0x4000);
    main.store(0x1008,4,20);main.store(0x100c,4,0x44);
    for(unsigned n=0;n<32;++n)main.store(0x2000+n,1,n+1);
    main.w(3,0x77);main.w(4,0x1000);main.w(5,1);hg::kernel_call(main);
    CHECK(main.r(2)==1 && main.dmac.channels[6].chcr==0x184 && sub.load(0x4000,4,false)==0);
    const std::array<std::uint32_t,4> expected_sif_descriptor{0x2000,0x4000,20,0x44};
    CHECK(main.boot.sif_dma_trace_cursor==1);
    CHECK(main.boot.sif_dma_trace[0].pc+4==main.pc);
    CHECK(main.boot.sif_dma_trace[0].count==1);
    CHECK(main.boot.sif_dma_trace[0].descriptors[0]==expected_sif_descriptor);
    main.w(3,0x76);main.w(4,1);hg::kernel_call(main);CHECK(main.r(2)==0);
    main.store(0x2000,4,0xffffffff); // Accepted command owns a snapshot of its padded payload.
    main.w(3,0x77);main.w(4,0x1000);main.w(5,1);
    hg::kernel_call(main);CHECK(main.r(2)==0 && main.boot.sif_transfer_id==1);
    while(hg::service_sif(main,sub)){}
    main.w(3,hg::sx32(0xffffff8au));main.w(4,1);main.boot.in_interrupt=true;
    hg::kernel_call(main);main.boot.in_interrupt=false;CHECK(main.r(2)==hg::sx32(0xffffffffu));
    for(unsigned n=0;n<32;++n)CHECK(sub.load(0x4000+n,1,false)==n+1);
    CHECK(sub.dmac.channel[2][2]==0x40000300 && main.dmac.status==0x40);
    sub.dmac.completed_channels=0;sub.store(0x1f801538,4,0x41000300);
    main.store(0x1000,4,0x3000);main.store(0x1004,4,0x5008);main.store(0x1008,4,4);main.store(0x100c,4,0);
    main.store(0x1010,4,0x2000);main.store(0x1014,4,0x6000);main.store(0x1018,4,16);main.store(0x101c,4,0x44);
    main.store(0x3000,4,0x12345678);main.w(3,hg::sx32(0xffffff89u));main.w(4,0x1000);main.w(5,2);
    main.boot.in_interrupt=true;hg::kernel_call(main);main.boot.in_interrupt=false;
    CHECK(main.r(2)==2);main.store(0x3000,4,0);
    unsigned steps=0;while(hg::service_sif(main,sub)){++steps;if(steps<4)CHECK(!sub.dmac.completed_channels);}
    CHECK(sub.load(0x5008,4,false)==0x12345678 && sub.load(0x6000,4,false)==0xffffffff);
    CHECK(sub.dmac.completed_channels==(1u<<10) && sub.dmac.channel[2][2]==0x40000300);
    // A single raw SIF DMA is a complete EE source transfer without a remote
    // ERT/INT_O request. It copies the payload and deliberately leaves the IOP
    // SIF1 destination channel armed for the next transfer.
    sub.dmac.completed_channels=0;sub.store(0x1f801538,4,0x41000300);
    main.dmac.status&=~(1u<<6);
    main.store(0x1000,4,0x2200);main.store(0x1004,4,0x884c0);main.store(0x1008,4,0x80);main.store(0x100c,4,0);
    for(unsigned n=0;n<0x80;++n)main.store(0x2200+n,1,0x80u^n);
    main.w(3,0x77);main.w(4,0x1000);main.w(5,1);hg::kernel_call(main);
    const auto raw_id=std::uint32_t(main.r(2));CHECK(raw_id!=0);
    while(hg::service_sif(main,sub)){}
    for(unsigned n=0;n<0x80;++n)CHECK(sub.load(0x884c0+n,1,false)==(0x80u^n));
    CHECK(sub.dmac.completed_channels==0 && sub.dmac.channel[2][2]==0x41000300);
    CHECK((main.dmac.status&(1u<<6))!=0);
    main.w(3,0x76);main.w(4,raw_id);hg::kernel_call(main);CHECK(main.r(2)==hg::sx32(0xffffffffu));
    // Larger raw transfers use the same backpressured transport and preserve
    // the entire submitted snapshot, including both ends of the payload.
    main.store(0x1000,4,0x100000);main.store(0x1004,4,0x136500);
    main.store(0x1008,4,0x2800);
    for(unsigned n=0;n<0x2800;++n)main.store(0x100000+n,1,(n*13+7)&255);
    main.w(3,0x77);main.w(4,0x1000);main.w(5,1);hg::kernel_call(main);
    main.store(0x100000,4,0);
    while(hg::service_sif(main,sub)){}
    for(unsigned n=0;n<0x2800;++n)CHECK(sub.load(0x136500+n,1,false)==((n*13+7)&255));
    CHECK(sub.dmac.channel[2][2]==0x41000300 && !sub.dmac.completed_channels);
    const auto large_id=main.boot.sif_transfer_id;
    const auto large_tag=main.load(0x60000,8,false);
    main.store(0x1008,4,0x1fff0);main.w(3,0x77);fault=false;
    try{hg::kernel_call(main);}catch(const hg::Fault&){fault=true;}
    CHECK(fault && main.boot.sif_transfer_id==large_id && main.load(0x60000,8,false)==large_tag);
    main.store(0x1008,4,0x10000);main.store(0x1010,4,0x100000);
    main.store(0x1014,4,0x150000);main.store(0x1018,4,0x10000);main.store(0x101c,4,0);
    main.w(3,0x77);main.w(5,2);fault=false;
    try{hg::kernel_call(main);}catch(const hg::Fault&){fault=true;}
    CHECK(fault && main.boot.sif_transfer_id==large_id && main.load(0x60000,8,false)==large_tag);
    const auto id=main.boot.sif_transfer_id;const auto tag=main.load(0x60000,8,false);
    main.store(0x101c,4,0x45);main.w(3,0x77);fault=false;
    try{hg::kernel_call(main);}catch(const hg::Fault&){fault=true;}
    CHECK(fault && main.boot.sif_transfer_id==id && main.load(0x60000,8,false)==tag && !(main.dmac.channels[6].chcr&0x100));
    main.boot.dma_handlers.push_back({5,0x4000,0,0x5678});
    main.dmac.status=0x20;main.dmac.mask=0x20;main.pc=0x5000;
    main.w(2,0x1122);main.w(29,0x9000);main.hi=0x3344;main.fpr[4]=0x5566;
    const auto prior_status=main.cp0_status;
    auto callback=[](hg::State& x,std::uint64_t) {
        if(x.pc!=0x4000 || !x.boot.in_interrupt || (x.cp0_status&1u) || x.r(4)!=5 || x.r(5)!=0x5678)
            throw std::runtime_error("bad EE IRQ entry context");
        const auto target=std::uint32_t(x.r(31));
        x.dmac.channels[5].chcr=0;x.w(3,hg::sx32(0xffffff88u));hg::kernel_call(x);
        x.store(0x7000,4,0xabcd);x.w(2,0);x.hi=0;x.fpr[4]=0;x.pc=target;
    };
    for(const auto disabled: {prior_status&~1u,prior_status|2u,prior_status|4u}) {
        main.cp0_status=disabled;
        CHECK(!hg::service_ee_dma_interrupt(main,callback));
        CHECK(main.dmac.status&0x20);
    }
    main.cp0_status=prior_status;
    CHECK(hg::service_ee_dma_interrupt(main,callback));
    CHECK(main.pc==0x5000 && main.r(2)==0x1122 && main.r(29)==0x9000);
    CHECK(main.hi==0x3344 && main.fpr[4]==0x5566 && main.cp0_status==prior_status);
    CHECK(main.load(0x7000,4,false)==0xabcd && main.dmac.channels[5].chcr==0x184);
    CHECK(!main.boot.in_interrupt && !(main.dmac.status&0x20));
    // A callback may span host slices without losing interrupted CPU context.
    // A second completion remains pending until the first callback returns.
    hg::EeDmaInterruptDispatcher dispatcher;
    main.dmac.status=0x20;
    unsigned callback_steps=0;
    auto incremental=[&](hg::State& x,std::uint64_t count) {
        CHECK(count==1 && x.boot.in_interrupt);
        ++callback_steps;x.w(2,0);x.hi=0;
        x.pc=callback_steps%3 ? x.pc+4 : std::uint32_t(x.r(31));
        return 0;
    };
    CHECK(dispatcher.service(main,incremental) && dispatcher.active());
    CHECK(main.pc==0x4004 && !(main.cp0_status&0x10000));
    main.dmac.status|=0x20;
    CHECK(dispatcher.service(main,incremental) && dispatcher.active());
    CHECK(dispatcher.service(main,incremental) && !dispatcher.active());
    CHECK(main.pc==0x5000 && main.r(2)==0x1122 && main.hi==0x3344);
    CHECK(main.cp0_status==prior_status && (main.dmac.status&0x20));
    CHECK(dispatcher.service(main,incremental,3) && !dispatcher.active());
    CHECK(callback_steps==6 && !(main.dmac.status&0x20));
    main.boot.semaphores.push_back({0,1,0,0,0});main.pc=0x8000;
    main.w(3,0x44);main.w(4,1);main.w(2,0x99);hg::kernel_call(main);
    CHECK(main.pc==0x8000 && main.r(2)==0x99);
    main.boot.in_interrupt=true;main.w(3,hg::sx32(0xffffffbd));hg::kernel_call(main);
    CHECK(main.boot.semaphores[0].count==1 && main.r(2)==0);
    main.boot.in_interrupt=false;main.pc=0x8000;main.w(3,0x44);hg::kernel_call(main);
    CHECK(main.pc==0x8004 && main.boot.semaphores[0].count==0 && main.r(2)==0);
    main.w(3,0x41);hg::kernel_call(main);CHECK(main.boot.semaphores[0].maximum==0);
    main.w(3,0x44);fault=false;
    try {hg::kernel_call(main);}catch(const hg::Fault&){fault=true;}
    CHECK(fault);
    main.store(0x7004,4,2);main.store(0x7008,4,1);
    main.w(3,0x40);main.w(4,0x7000);hg::kernel_call(main);
    CHECK(main.r(2)==1 && main.boot.semaphores.size()==1 && main.boot.semaphores[0].count==1);
    // A blocked root yields to a worker; a signal reserves exactly one token
    // and resumes its saved syscall and guest kernel return stack.
    hg::State scheduled;scheduled.boot.enabled=true;scheduled.boot.thread_ready=true;
    scheduled.boot.threads.resize(2);scheduled.boot.current_thread=1;
    scheduled.boot.threads[0].status=1;scheduled.boot.threads[0].priority=1;
    auto& worker=scheduled.boot.threads[1];worker.status=2;worker.priority=10;worker.initialized=true;
    worker.context.pc=0x9000;scheduled.pc=0x8000;
    scheduled.boot.semaphores.push_back({0,1,0,0,0});
    // Initialize the native syscall table before adding an outstanding frame.
    scheduled.w(3,0x2f);hg::kernel_call(scheduled);scheduled.pc=0x8000;
    scheduled.boot.returns.push_back({0x1234,0x5678});
    scheduled.w(3,0x44);scheduled.w(4,1);hg::kernel_call(scheduled);
    CHECK(scheduled.boot.threads[0].status==4 && scheduled.pc==0x8000);
    bool isolated=false;
    hg::service_ee_thread(scheduled,[&](hg::State& x,std::uint64_t) {
        isolated=x.boot.current_thread==2 && x.boot.returns.empty() && x.pc==0x9000;
        x.w(3,0x42);x.w(4,1);hg::kernel_call(x);
    });
    CHECK(isolated && scheduled.boot.semaphores[0].count==0 && scheduled.boot.threads[0].granted_semaphore==1);
    hg::service_ee_thread(scheduled,[](hg::State& x,std::uint64_t){hg::kernel_call(x);});
    CHECK(scheduled.boot.current_thread==1 && scheduled.pc==0x8004 && scheduled.r(2)==0);
    CHECK(scheduled.boot.returns.size()==1 && scheduled.boot.returns[0].pc==0x1234);
    CHECK(!scheduled.boot.threads[0].granted_semaphore && scheduled.boot.semaphores[0].count==0);
    // Equal-priority rotation changes the next runnable thread, not its CPU
    // registers; the former thread resumes from its syscall continuation.
    scheduled.boot.threads[0].priority=10;
    scheduled.w(3,0x2b);scheduled.w(4,10);hg::kernel_call(scheduled);
    bool rotated=false;
    hg::service_ee_thread(scheduled,[&](hg::State& x,std::uint64_t){rotated=x.boot.current_thread==2;});
    CHECK(rotated && scheduled.boot.threads[0].status==2);
    scheduled.boot.threads[0].status=4;scheduled.boot.threads[1].status=4;
    CHECK(!hg::service_ee_thread(scheduled,[](hg::State&,std::uint64_t){throw std::runtime_error("idle guest ran");}));
    CHECK(scheduled.boot.current_thread==0);
    scheduled.boot.in_interrupt=true;scheduled.w(3,0x2f);hg::kernel_call(scheduled);
    CHECK(scheduled.r(2)==0 && scheduled.boot.threads[0].status==4 && scheduled.boot.threads[1].status==4);
    scheduled.boot.in_interrupt=false;
    return 0;
}
