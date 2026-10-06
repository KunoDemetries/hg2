#include "hg/gs_memory.hpp"
#include "hg/gs_raster_pages.hpp"
#include <algorithm>
#include <stdexcept>
#include <iostream>
#define CHECK(x) do {if(!(x))throw std::runtime_error(#x);}while(0)
namespace {
struct Observer: hg::GsMemoryObserver {
    unsigned calls=0,writes=0,forgets=0;
    bool pending=false,fail=false;
    std::bitset<512> written_pages;
    std::vector<bool> access_order;
    void access(std::uint32_t* p,std::size_t first,std::size_t count,bool write) override {
        ++calls;writes+=write;
        access_order.push_back(write);
        if(write&&count)for(auto page=first/2048;page<=(first+count-1)/2048;++page)written_pages.set(page);
        if(fail)throw std::runtime_error("observer fault");
        if(pending&&first<=3&&first+count>3){p[3]=0xabcdef12;pending=false;}
    }
    void forget(std::uint32_t*) noexcept override{++forgets;}
};
}
int main(){
    {
        hg::GsRegisterState gs;gs.vram.resize(1024*1024);
        unsigned cases=0;
        for(unsigned fp:{0u,1u,2u,10u,0x31u})for(unsigned zp:{0u,1u,2u,10u})
        for(unsigned width:{64u,320u,1024u,4032u})for(unsigned base:{0u,211u,510u})
        for(unsigned seed=0;seed<8;++seed) {
            const unsigned context=seed&1;
            gs.value[0x4c+context]=base|(std::uint64_t(width/64)<<16)|(std::uint64_t(fp)<<24);
            gs.value[0x4e + context]=((base+17)&511)|(std::uint64_t(zp)<<24);
            const int left=int((seed*293)%2048),top=int((seed*457)%2048);
            const int right=std::min(2048,left+137),bottom=std::min(2048,top+131);
            const auto pages=hg::gs_raster_write_pages(gs,context,left,top,right,bottom);
            std::bitset<512> expected;
            const auto fa=fp==2?hg::GsRegisterState::psmct16_word:fp==10?hg::GsRegisterState::psmct16s_word:
                fp==0x31?hg::GsRegisterState::psmz32_word:hg::GsRegisterState::psmct32_word;
            const auto za=zp==2?hg::GsRegisterState::psmz16_word:zp==10?hg::GsRegisterState::psmz16s_word:hg::GsRegisterState::psmz32_word;
            for(int y=top;y<bottom;++y)for(int x=left;x<right;++x){
                expected.set(fa(base*2048,width,x,y)/2048);
                expected.set(za(((base+17)&511)*2048,width,x,y)/2048);
            }
            CHECK(pages==expected);++cases;
        }
        CHECK(cases==1920);
        CHECK(hg::gs_raster_write_pages(gs,2,0,0,1,1).all());
        CHECK(hg::gs_raster_write_pages(gs,0,-1,0,1,1).all());
        CHECK(hg::gs_raster_write_pages(gs,0,0,0,2049,1).all());
        gs.value[0x4c]=0;CHECK(hg::gs_raster_write_pages(gs,0,0,0,1,1).all());
        auto tracked=std::make_shared<Observer>();gs.vram.observe(tracked);
        std::bitset<512> pages;pages.set(0).set(1).set(511);
        tracked->pending=true;
        {
            hg::GsCpuMemoryScope scope(gs.vram,pages);
            CHECK(gs.vram[3]==0xabcdef12);
            const auto calls=tracked->calls;gs.vram[3]=9;gs.vram[511*2048]=10;
            CHECK(tracked->calls==calls);
        }
        CHECK(tracked->written_pages==pages);
        CHECK(tracked->access_order==std::vector<bool>({false,true,true}));
        const auto calls=tracked->calls;
        CHECK(static_cast<const hg::GsLocalMemory&>(gs.vram)[3]==9&&tracked->calls==calls+1);
        try{hg::GsCpuMemoryScope scope(gs.vram,pages);throw std::runtime_error("raster fault");}catch(const std::runtime_error&){}
        const auto restored=tracked->calls;
        CHECK(static_cast<const hg::GsLocalMemory&>(gs.vram)[3]==9&&tracked->calls==restored+1);
        tracked->fail=true;bool failed=false;
        try{hg::GsCpuMemoryScope scope(gs.vram,pages);}catch(const std::runtime_error&){failed=true;}
        CHECK(failed);tracked->fail=false;
        CHECK(static_cast<const hg::GsLocalMemory&>(gs.vram)[3]==9);
        std::cout<<cases<<" raster write-page coverage cases passed\n";
    }
    auto observer=std::make_shared<Observer>();
    {
        hg::GsLocalMemory memory;memory.resize(16);memory.observe(observer);
        observer->pending=true;const auto& read=memory;
        CHECK(read[3]==0xabcdef12&&observer->writes==0);
        memory[3]=7;CHECK(observer->writes==1&&read[3]==7);
        observer->pending=true;auto copy=memory;CHECK(copy[3]==0xabcdef12);
        const auto calls=observer->calls;copy[3]=4;CHECK(observer->calls==calls&&read[3]==0xabcdef12);
        observer->pending=true;
        {hg::GsCpuMemoryScope scope(memory);const auto before=observer->calls;
         for(unsigned i=0;i<16;++i)memory[i]=i;CHECK(observer->calls==before);}
        CHECK(read[3]==3);CHECK(observer->calls>calls);
        try{hg::GsCpuMemoryScope scope(memory);throw std::runtime_error("CPU fault");}catch(const std::runtime_error&){}
        const auto restored=observer->calls;CHECK(read[3]==3&&observer->calls==restored+1);
        observer->pending=true;auto moved=std::move(memory);CHECK(memory.empty());
        CHECK(static_cast<const hg::GsLocalMemory&>(moved)[3]==0xabcdef12);
        observer->fail=true;bool failed=false;
        try{hg::GsCpuMemoryScope scope(moved);}catch(const std::runtime_error&){failed=true;}
        CHECK(failed);observer->fail=false;CHECK(static_cast<const hg::GsLocalMemory&>(moved)[3]==0xabcdef12);
        moved.resize(32);CHECK(moved[3]==0xabcdef12);
        const auto before=observer->calls;std::fill(copy.begin(),copy.end(),0);CHECK(observer->calls==before);
        CHECK(moved!=copy);
        observer->pending=true;
        const auto span_calls=observer->calls,span_writes=observer->writes;
        auto* span=moved.write_span(2,4);
        CHECK(span[1]==0xabcdef12&&observer->calls==span_calls+1&&observer->writes==span_writes+1);
        span[0]=99;CHECK(moved.backend_data()[2]==99);
        for(auto first:{std::size_t(33),std::size_t(31)}){
            const auto before_fault=observer->calls;bool rejected=false;
            try{moved.write_span(first,2);}catch(const std::out_of_range&){rejected=true;}
            CHECK(rejected&&observer->calls==before_fault);
        }
        observer->fail=true;bool span_failed=false;
        try{moved.write_span(2,4);}catch(const std::runtime_error&){span_failed=true;}
        CHECK(span_failed&&moved.backend_data()[2]==99);observer->fail=false;
        observer->pending=true;
        const auto read_calls=observer->calls,read_writes=observer->writes;
        const auto* read_span=static_cast<const hg::GsLocalMemory&>(moved).read_span(2,4);
        CHECK(read_span[1]==0xabcdef12&&observer->calls==read_calls+1&&observer->writes==read_writes);
        bool read_failed=false;
        try{static_cast<const hg::GsLocalMemory&>(moved).read_span(31,2);}catch(const std::out_of_range&){read_failed=true;}
        CHECK(read_failed&&observer->calls==read_calls+1);
    }
    CHECK(observer->forgets==2);std::cout<<"GS memory observer/copy/move/scope checks passed\n";
}
