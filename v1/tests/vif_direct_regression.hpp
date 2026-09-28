#pragma once
#include "hg/vif.hpp"
#include "gif_stream_regression.hpp"
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace vif_direct_regression {
using Qword=gif_stream_regression::Qword;
inline std::uint32_t word(const std::vector<std::uint8_t>& b,std::size_t at) {
    return std::uint32_t(b.at(at))|(std::uint32_t(b.at(at+1))<<8)|
           (std::uint32_t(b.at(at+2))<<16)|(std::uint32_t(b.at(at+3))<<24);
}
// Frozen serial reference: one GIF submit per qword after the original complete
// VIF command check. Never shares the proposed complete-packet fast path.
inline void reference(hg::Vif1Path& v,hg::GifPath& gif,hg::GsRegisterState& gs) {
    if(v.pending.size()>hg::Vif1Path::max_pending)throw std::runtime_error("VIF1 packet exceeds bounded path capacity");
    std::size_t cursor=0;
    while(v.pending.size()-cursor>=4) {
        const auto code=word(v.pending,cursor);
        if(code&0x80000000u)throw std::runtime_error("VIF1 interrupt-control VIFcode requires interrupt delivery");
        const auto cmd=std::uint8_t(code>>24)&0x7f;
        const auto immediate=std::uint16_t(code);
        if(cmd==0){cursor+=4;continue;}
        if(cmd!=0x50&&cmd!=0x51)throw std::logic_error("unsupported synthetic DIRECT fixture command");
        const std::size_t units=immediate?immediate:65536,bytes=units*16;
        if((v.pending_phase+cursor+4)%16)throw std::runtime_error("VIF1 DIRECT payload is not GIF-qword aligned");
        if(v.pending.size()-cursor<4+bytes)break;
        for(std::size_t n=0;n<units;++n) {
            const auto at=cursor+4+n*16;
            const Qword q{std::uint64_t(word(v.pending,at))|(std::uint64_t(word(v.pending,at+4))<<32),
                          std::uint64_t(word(v.pending,at+8))|(std::uint64_t(word(v.pending,at+12))<<32)};
            gif_stream_regression::reference_submit(gif,q,gs);
        }
        cursor+=4+bytes;
    }
    if(cursor) {v.pending.erase(v.pending.begin(),v.pending.begin()+cursor);v.pending_phase=(v.pending_phase+cursor)%16;}
}
inline void append32(std::vector<std::uint8_t>& b,std::uint32_t value) {
    for(unsigned n=0;n<4;++n)b.push_back(std::uint8_t(value>>(8*n)));
}
inline std::vector<std::uint8_t> encode(const std::vector<Qword>& payload,unsigned command,unsigned nops) {
    std::vector<std::uint8_t> bytes(nops*4,0);
    append32(bytes,(command<<24)|unsigned(payload.size()&0xffff));
    for(const auto q:payload) {
        append32(bytes,std::uint32_t(q.low));append32(bytes,std::uint32_t(q.low>>32));
        append32(bytes,std::uint32_t(q.high));append32(bytes,std::uint32_t(q.high>>32));
    }
    return bytes;
}
inline bool same_vif(const hg::Vif1Path& a,const hg::Vif1Path& b) {
    return a.pending==b.pending&&a.pending_phase==b.pending_phase&&a.row==b.row&&a.column==b.column&&
           a.vu_mem==b.vu_mem&&a.vu_mem_defined==b.vu_mem_defined&&a.vu_micro_mem==b.vu_micro_mem&&
           a.cycle==b.cycle&&a.mode==b.mode&&a.mask==b.mask&&a.top==b.top&&a.tops==b.tops&&
           a.vu1.issue_cycle==b.vu1.issue_cycle&&a.vu1.tpc==b.vu1.tpc;
}
inline bool run_case(std::vector<std::uint8_t> bytes,const std::vector<Qword>& prefix,
                     const hg::GsRegisterState& initial,unsigned phase,std::size_t chunk,
                     std::uint64_t& checkpoints,bool full_gif=false) {
    auto a=std::make_unique<hg::Vif1Path>(),b=std::make_unique<hg::Vif1Path>();
    auto x=std::make_unique<hg::GsRegisterState>(initial),y=std::make_unique<hg::GsRegisterState>(initial);
    hg::GifPath ga,gb;
    a->pending_phase=b->pending_phase=phase;
    a->vu_mem[7]=b->vu_mem[7]=0xabcdef12;a->vu1.issue_cycle=b->vu1.issue_cycle=0x12345678;
    for(const auto q:prefix) {
        ga.submit_qword(q.low,q.high,*x);gif_stream_regression::reference_submit(gb,q,*y);
    }
    if(full_gif){ga.pending.resize(hg::GifPath::max_pending);gb.pending=ga.pending;}
    for(std::size_t at=0;at<bytes.size();) {
        const auto end=std::min(bytes.size(),at+chunk);
        a->pending.insert(a->pending.end(),bytes.begin()+at,bytes.begin()+end);
        b->pending.insert(b->pending.end(),bytes.begin()+at,bytes.begin()+end);at=end;
        const auto af=gif_stream_regression::fault([&]{a->process_pending(ga,*x);});
        const auto bf=gif_stream_regression::fault([&]{reference(*b,gb,*y);});
        ++checkpoints;
        if(af!=bf||!same_vif(*a,*b)||ga.pending!=gb.pending||!gif_stream_regression::same_state(*x,*y)) {
            std::cerr<<"VIF DIRECT mismatch checkpoint="<<checkpoints<<" phase="<<phase
                     <<" chunk="<<chunk<<" actual='"<<af<<"' expected='"<<bf<<"'\n";return false;
        }
        if(!af.empty())break;
    }
    const auto af=gif_stream_regression::fault([&]{x->rasterize_pending_draws();});
    const auto bf=gif_stream_regression::fault([&]{y->rasterize_pending_draws();});
    ++checkpoints;
    if(af!=bf||!gif_stream_regression::same_state(*x,*y))return false;
    ga.write_control(1);gb.write_control(1);
    return ga.pending==gb.pending&&gif_stream_regression::same_state(*x,*y);
}
inline int run() {
    const auto initial=gif_stream_regression::initial_state();
    std::uint64_t checkpoints=0,cases=0;
    const auto test=[&](const std::vector<Qword>& payload,const std::vector<Qword>& prefix,
                        const hg::GsRegisterState& gs,unsigned nops=0,unsigned phase=12,
                        unsigned command=0x50) {
        for(const auto chunk:{std::size_t(16),std::size_t(53),std::size_t(1048600)}) {
            ++cases;if(!run_case(encode(payload,command,nops),prefix,gs,phase,chunk,checkpoints))return false;
        }
        return true;
    };
    const auto tag=gif_stream_regression::tag;
    // Several complete packets in one command; all NREG encodings, odd REGLIST,
    // empty tags, PRE, leading NOPs, partial command input and both DIRECT kinds.
    for(unsigned format=0;format<2;++format)for(unsigned nreg=1;nreg<=16;++nreg)
    for(unsigned loops:{0u,1u,3u})for(bool pre:{false,true}) {
        std::uint64_t desc=0;const unsigned safe[]={15,10,1,3};
        for(unsigned i=0;i<nreg;++i)desc|=std::uint64_t(safe[i%4])<<(4*i);
        std::vector<Qword> q{{tag(format,nreg,loops,pre,true),desc}};
        const auto count=format==0?loops*nreg:(loops*nreg+1)/2;
        for(unsigned i=0;i<count;++i)q.push_back({0x3f80000080402010ull+i,0x012345670000003dull+i});
        q.push_back({tag(0,1,0,true,true),0});q.push_back({tag(0,1,1,false,true),10});q.push_back({0,0x12340000});
        const unsigned nops=(format+loops)%4;
        if(!test(q,{},initial,nops,(12+16-4*nops)%16,pre?0x51:0x50))return 1;
    }
    // All descriptors, including unsupported forms and partial GS writes before
    // failure. A succeeding first packet must remain committed on later failure.
    for(unsigned format=0;format<2;++format)for(unsigned desc=0;desc<16;++desc) {
        std::vector<Qword> q{{tag(0,1,1,false,true),10},{0,0x99880000},
                            {tag(format,2,2,true,true),10ull|(std::uint64_t(desc)<<4)}};
        for(unsigned n=0;n<(format==0?4u:2u);++n)q.push_back({0x0000000800000010ull+n,0x3d});
        if(!test(q,{},initial))return 1;
    }
    // An existing GIF suffix forces serial completion before the next bulk packet.
    const std::vector<Qword> prefix{{tag(0,1,3,false,true),10},{0,1}};
    const std::vector<Qword> continuation{{0,2},{0,3},{tag(0,1,1,false,true),10},{0,4}};
    if(!test(continuation,prefix,initial))return 1;
    // Multi-tag EOP packet, all-NOP tag and empty PRE tag retain original Q reset rules.
    const std::vector<Qword> multi{{tag(0,1,0,true,false),0},{tag(0,1,2,false,false),15},
        {123,456},{789,1011},{tag(0,2,1,true,true),0x12},{0,0x40000000},{0x80808080,0}};
    if(!test(multi,{},initial))return 1;
    for(unsigned format:{2u,3u})for(unsigned psm:{0u,1u,2u,0x13u}) {
        auto gs=initial;gs.write_ad(0x50,(1ull<<48)|(std::uint64_t(psm)<<56));
        gs.write_ad(0x51,0);gs.write_ad(0x52,8ull|(2ull<<32));gs.write_ad(0x53,0);
        std::vector<Qword> q{{tag(format,1,8,false,true),0}};
        for(unsigned n=0;n<8;++n)q.push_back({0x1122334455667788ull+n,0xabcdef9080706050ull-n});
        if(!test(q,{},gs)||!test(q,{},initial))return 1;
    }
    const auto xy=[](unsigned x,unsigned y){return std::uint64_t(x*16)|(std::uint64_t(y*16)<<16);};
    for(bool invalid:{false,true}) {
        std::vector<Qword> q{{tag(0,1,invalid?6:5,false,true),14},{6,0},{0x3f80000080443322ull,1},
                             {xy(1,1),5},{xy(8,8),5},{0,0x61}};
        if(invalid)q.push_back({123,0x80});
        if(!test(q,{},initial))return 1;
    }
    if(!test({{tag(0,1,3,false,true),14},{0xffffffff12345678ull,0x60},{123,0x3d},{0xffffffff87654321ull,0x60}}, {},initial))return 1;
    // Incomplete GIF suffix remains pending, even though VIF DIRECT itself ends.
    if(!test({{tag(0,1,20,false,true),10},{0,1},{0,2}}, {},initial))return 1;
    const std::vector<Qword> tiny{{tag(0,1,1,false,true),10},{0,0x12340000}};
    for(unsigned phase=0;phase<16;++phase)for(unsigned truncated=0;truncated<3;++truncated) {
        auto bytes=encode(tiny,0x50,0);
        if(truncated==1)bytes.resize(7);if(truncated==2)bytes.resize(3);
        ++cases;if(!run_case(bytes,{},initial,phase,53,checkpoints))return 1;
    }
    {auto bytes=encode(tiny,0x50,0);bytes[3]|=0x80;++cases;
        if(!run_case(bytes,{},initial,12,53,checkpoints))return 1;}
    // Large known payloads can straddle DIRECT commands and existing GIF suffixes.
    // The completing qword must retain full decode, partial-state and fault order.
    for(unsigned format=0;format<4;++format)for(bool late_fault:{false,true}) {
        auto gs=initial;gs.write_ad(0x50,1ull<<48);gs.write_ad(0x51,0);
        gs.write_ad(0x52,64ull|(64ull<<32));gs.write_ad(0x53,0);
        std::vector<Qword> q{{tag(0,1,0,true,false),0},{tag(format,1,1023,false,true),15}};
        const unsigned payload_count=format==1?512:1023;
        for(unsigned n=0;n<payload_count;++n)q.push_back({0x1234567887654321ull+n,0xfedcba9080706050ull-n});
        if(late_fault){q.push_back({tag(0,1,1,false,true),14});q.push_back({123,0x80});}
        for(const auto cut:{std::size_t(0),std::size_t(1),std::size_t(2),std::size_t(17),std::size_t(511)}) {
            const std::vector<Qword> old_prefix(q.begin(),q.begin()+cut),tail(q.begin()+cut,q.end());
            for(const auto chunk:{std::size_t(257),std::size_t(8192),std::size_t(1048600)}) {
                ++cases;if(!run_case(encode(tail,0x50,0),old_prefix,gs,12,chunk,checkpoints))return 1;
            }
        }
        const std::vector<Qword> first(q.begin(),q.begin()+257),second(q.begin()+257,q.end());
        auto bytes=encode(first,0x50,0);const auto rest=encode(second,0x51,3);
        bytes.insert(bytes.end(),rest.begin(),rest.end());
        for(const auto chunk:{std::size_t(257),std::size_t(8192),std::size_t(1048600)}) {
            ++cases;if(!run_case(bytes,{},gs,12,chunk,checkpoints))return 1;
        }
    }
    // Stop at capacity even when a known tag declares much more input. The
    // original first overflowing qword, not an entire batch, must remain queued.
    {
        std::vector<Qword> nearly_full(hg::GifPath::max_pending/16-4,Qword{0,0});
        nearly_full[0]={tag(0,16,32767,false,false),0xffffffffffffffffull};
        nearly_full[1+32767*16]={tag(2,1,64,false,true),0};
        const std::vector<Qword> extra(16,Qword{0x12345678,0x87654321});
        ++cases;if(!run_case(encode(extra,0x50,0),nearly_full,initial,12,1048600,checkpoints))return 1;
    }
    // Zero immediate means 65536 qwords, including complete and truncated cases.
    {std::vector<Qword> q(65536,Qword{0x8000,0});++cases;
        if(!run_case(encode(q,0x51,0),{},initial,12,1048600,checkpoints))return 1;}
    {std::vector<std::uint8_t> bytes;append32(bytes,0x50000000u);++cases;
        if(!run_case(bytes,{},initial,12,16,checkpoints))return 1;}
    // Size faults preserve the exact overflowing suffix, not a whole extra batch.
    {++cases;if(!run_case(encode(tiny,0x50,0),{},initial,12,1048600,checkpoints,true))return 1;}
    {std::vector<std::uint8_t> bytes(hg::Vif1Path::max_pending+1,0);++cases;
        if(!run_case(bytes,{},initial,0,bytes.size(),checkpoints))return 1;}
    std::cout<<"VIF DIRECT transport: "<<cases<<" serial-reference cases / "<<checkpoints
             <<" exact state/fault/partial-packet checkpoints passed\n";
    return 0;
}
}
