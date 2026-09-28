#pragma once
#include "hg/vu_specialized.hpp"
#include <array>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <string>
#if defined(_M_X64) || defined(__SSE2__)
#include <xmmintrin.h>
#endif

namespace hg_test {
inline bool same_vu_specialization_state(const hg::Vu1State& a,const hg::Vu1State& b) {
    return a.vf==b.vf&&a.vf_defined==b.vf_defined&&a.vi==b.vi&&
        a.acc==b.acc&&a.acc_defined==b.acc_defined&&a.clip==b.clip&&
        a.status==b.status&&a.mac==b.mac&&a.q==b.q&&a.i==b.i&&a.p==b.p&&
        a.pending_q==b.pending_q&&a.pending_p==b.pending_p&&
        a.q_cycles_remaining==b.q_cycles_remaining&&a.p_cycles_remaining==b.p_cycles_remaining&&
        a.q_pending==b.q_pending&&a.p_pending==b.p_pending&&a.tpc==b.tpc&&
        a.issue_cycle==b.issue_cycle&&a.vf_ready==b.vf_ready&&a.vi_ready==b.vi_ready;
}

inline bool run_vu_specialization_regression() {
    struct Case {
        unsigned destination,source,other,mask,broadcast;
        void (*execute)(hg::Vu1State&);
        void (*compact)(hg::Vu1State&,unsigned,unsigned,unsigned);
    };
#define HG_VU_CASE(D,S,O,M,B) {D,S,O,M,B,&hg::vu_aot_multiply<D,S,O,M,B>,nullptr}
#define HG_VU_COMPACT_CASE(D,S,O,M,B) {D,S,O,M,B,&hg::vu_aot_multiply<D,S,O,M,B>,&hg::vu_masked_multiply<M,B>}
#define HG_VU_BROADCASTS(D,S,O,M) HG_VU_COMPACT_CASE(D,S,O,M,0),HG_VU_COMPACT_CASE(D,S,O,M,1),HG_VU_COMPACT_CASE(D,S,O,M,2),HG_VU_COMPACT_CASE(D,S,O,M,3)
#define HG_VU_MASKS(D,S,O) HG_VU_BROADCASTS(D,S,O,14),HG_VU_BROADCASTS(D,S,O,15)
    const Case cases[]={
        HG_VU_MASKS(3,1,2),HG_VU_MASKS(1,1,2),HG_VU_MASKS(2,1,2),HG_VU_MASKS(1,1,1),
        HG_VU_MASKS(0,1,2),HG_VU_MASKS(3,0,2),HG_VU_MASKS(3,1,0),HG_VU_MASKS(3,0,0),
        HG_VU_MASKS(31,31,31),HG_VU_MASKS(3,32,2),HG_VU_MASKS(3,1,32),HG_VU_MASKS(32,1,2),
        HG_VU_CASE(3,1,2,14,4),HG_VU_CASE(3,32,32,15,4),
        HG_VU_CASE(32,32,2,0,0),HG_VU_CASE(32,1,32,0,0),
        HG_VU_CASE(32,1,2,16,0),HG_VU_CASE(3,1,2,31,3),
        HG_VU_CASE(1,1,2,8,1),HG_VU_CASE(2,1,2,1,3)
    };
#undef HG_VU_MASKS
#undef HG_VU_BROADCASTS
#undef HG_VU_COMPACT_CASE
#undef HG_VU_CASE
    constexpr std::array<std::uint32_t,24> edges{
        0u,0x80000000u,1u,0x007fffffu,0x807fffffu,0x00800000u,0x80800000u,0x00800001u,
        0x3f000000u,0xbf000000u,0x3f7fffffu,0x3f800000u,0xbf800000u,0x3f800001u,
        0x40000000u,0xc0000000u,0x4f000000u,0xcf000000u,0x7f7fffffu,0x7f800000u,
        0x7fffffffu,0xff7fffffu,0xff800000u,0xffffffffu};
    std::uint32_t seed=0xd15ea5edu;
    const auto next=[&](){seed=seed*1664525u+1013904223u;return seed;};
    const auto next64=[&](){const auto hi=next();const auto lo=next();return (std::uint64_t(hi)<<32)|lo;};
#if defined(_M_X64) || defined(__SSE2__)
    struct RestoreCsr {unsigned value=_mm_getcsr();~RestoreCsr(){_mm_setcsr(value);}} restore;
    constexpr unsigned modes=16;
#else
    constexpr unsigned modes=1;
#endif
    std::uint64_t comparisons=0,faults=0,compact_comparisons=0,compact_faults=0;
    for(unsigned mode=0;mode<modes;++mode) {
#if defined(_M_X64) || defined(__SSE2__)
        const auto csr=(restore.value&~0xe040u)|((mode&3u)<<13)|((mode&4u)?0x8000u:0u)|((mode&8u)?0x40u:0u);
        _mm_setcsr(csr);
#endif
        for(std::size_t index=0;index<std::size(cases);++index)for(unsigned sample=0;sample<512;++sample) {
            const auto& test=cases[index];
            hg::Vu1State expected;
            for(unsigned reg=0;reg<32;++reg) {
                for(auto& bits:expected.vf[reg])bits=(sample&1)?next():edges[next()%edges.size()];
                const unsigned defined=sample<256?15u:((sample>>((reg&1)?0:4))&15u);
                expected.vf_defined[reg]=std::uint8_t((next()&0xf0u)|defined);
                for(auto& ready:expected.vf_ready[reg])ready=next64();
            }
            for(auto& value:expected.vi)value=std::uint16_t(next());
            for(auto& ready:expected.vi_ready)ready=next64();
            for(auto& value:expected.acc)value=next();
            expected.acc_defined=std::uint8_t(next());
            expected.clip=next();expected.status=next();expected.mac=next();
            expected.q=next();expected.p=next();expected.i=next();
            expected.pending_q=next();expected.pending_p=next();
            expected.q_pending=(sample&1)!=0;expected.p_pending=(sample&2)!=0;
            expected.q_cycles_remaining=next();expected.p_cycles_remaining=next();
            expected.tpc=std::uint16_t(next());
            expected.issue_cycle=sample%7?(next64()):~std::uint64_t(0)-sample;
            auto actual=expected,compact=expected;
            std::string expected_error,actual_error,compact_error;
            try {expected.multiply_vector(test.destination,test.source,test.other,test.mask,test.broadcast);}
            catch(const std::runtime_error& error){expected_error=error.what();}
            try {test.execute(actual);}
            catch(const std::runtime_error& error){actual_error=error.what();}
            if(expected_error!=actual_error||!same_vu_specialization_state(expected,actual)) {
                std::cerr<<"Static VU MUL mismatch case="<<index<<" sample="<<sample<<" mode="<<mode
                         <<" reference_error="<<expected_error<<" candidate_error="<<actual_error<<'\n';
                return false;
            }
            if(test.compact) {
                try {test.compact(compact,test.destination,test.source,test.other);}
                catch(const std::runtime_error& error){compact_error=error.what();}
                if(expected_error!=compact_error||!same_vu_specialization_state(expected,compact)) {
                    std::cerr<<"Compact VU MUL mismatch case="<<index<<" sample="<<sample<<" mode="<<mode
                             <<" reference_error="<<expected_error<<" candidate_error="<<compact_error<<'\n';
                    return false;
                }
                ++compact_comparisons;if(!compact_error.empty())++compact_faults;
            }
#if defined(_M_X64) || defined(__SSE2__)
            if(_mm_getcsr()!=csr){std::cerr<<"Static/compact VU MUL changed host MXCSR\n";return false;}
#endif
            ++comparisons;if(!expected_error.empty())++faults;
        }
    }
    std::cout<<"Static VU MUL: "<<std::size(cases)<<" operand specializations / "<<comparisons
             <<" exact state comparisons / "<<faults<<" matching faults across "<<modes<<" host modes; compact="
             <<compact_comparisons<<" exact state comparisons / "<<compact_faults<<" matching faults\n";
    return true;
}
} // namespace hg_test
