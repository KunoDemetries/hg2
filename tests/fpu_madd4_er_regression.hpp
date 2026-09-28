#pragma once
#include "hg/fpu.hpp"
#include "hg/fpu_add4.hpp"
#include <array>
#include <cstdint>
#include <iostream>
#if defined(_M_X64) || defined(__x86_64__)
#include <xmmintrin.h>
#endif

namespace hg { void fpu_madd4_avx2(const std::uint32_t*,std::uint32_t,const std::uint32_t*,std::uint32_t*,unsigned&,unsigned&); }

inline bool madd_er_regression() {
    const bool supported=hg::fpu_madd4_er_available();
    std::array<std::uint32_t,4> a{},acc{},out{};
    unsigned under=17,over=23;
    if(!supported) {
        const auto saved=out;
        if(hg::try_fpu_madd4_er(a,0,acc,out,under,over)||out!=saved||under!=17||over!=23)return false;
        std::cout<<"MADD embedded-rounding: unavailable CPU/OS, guarded no-op fallback passed\n";
        return true;
    }
    std::uint64_t cases=0,accepted=0,rejected=0;
    const auto check=[&](const std::array<std::uint32_t,4>& source,std::uint32_t scalar,
                         const std::array<std::uint32_t,4>& accumulator) {
        std::array<std::uint32_t,4> expected{},actual{0x12345678,0x89abcdef,0xaabbccdd,0xfeedface};
        const auto sentinel=actual;
        unsigned eu=0,eo=0,u=17,o=23;
        for(unsigned lane=0;lane<4;++lane) {
            const auto p=hg::Fpu::product(source[lane],scalar);
            hg::Fpu f;expected[lane]=f.add(accumulator[lane],p.bits);
            if(p.underflow||(f.control&0x4000))eu|=1u<<lane;
            if(p.overflow||(f.control&0x8000))eo|=1u<<lane;
        }
#if defined(_M_X64) || defined(__x86_64__)
        const auto csr=_mm_getcsr();
#endif
        const bool ok=hg::try_fpu_madd4_er(source,scalar,accumulator,actual,u,o);
#if defined(_M_X64) || defined(__x86_64__)
        if(_mm_getcsr()!=csr){std::cerr<<"MADD ER changed MXCSR\n";return false;}
#endif
        ++cases;
        if(ok) {
            ++accepted;
            if(actual!=expected||u!=eu||o!=eo) {
                std::cerr<<"MADD ER mismatch case="<<cases<<" scalar="<<std::hex<<scalar<<std::dec<<'\n';
                for(unsigned lane=0;lane<4;++lane)std::cerr<<std::hex<<source[lane]<<' '<<accumulator[lane]<<' '<<actual[lane]<<' '<<expected[lane]<<std::dec<<'\n';
                return false;
            }
        } else {
            ++rejected;
            if(actual!=sentinel||u!=17||o!=23){std::cerr<<"MADD ER rejection changed outputs\n";return false;}
        }
        // Independently keep comparing the already-retained integer AVX2 reference.
        std::array<std::uint32_t,4> old{};unsigned old_u,old_o;
        hg::fpu_madd4_avx2(source.data(),scalar,accumulator.data(),old.data(),old_u,old_o);
        if(old!=expected||old_u!=eu||old_o!=eo)return false;
        return true;
    };
#if defined(_M_X64) || defined(__x86_64__)
    struct RestoreMxcsr {unsigned value;~RestoreMxcsr(){_mm_setcsr(value);}} restore{_mm_getcsr()};
#endif
    for(unsigned rounding=0;rounding<4;++rounding)for(unsigned flush=0;flush<4;++flush) {
#if defined(_M_X64) || defined(__x86_64__)
        // Mask exceptions, vary rounding/FTZ/DAZ, and retain arbitrary preexisting flags.
        _mm_setcsr(0x1fa1u|(rounding<<13)|((flush&1)?0x8000u:0)|((flush&2)?0x40u:0));
#endif
        std::uint64_t random=0x2dd40101abcdef01ull;
        const auto next=[&]() {random=random*6364136223846793005ull+1442695040888963407ull;return std::uint32_t(random>>32);};
        for(unsigned index=0;index<65536;++index) {
            std::uint32_t b=next();
            for(unsigned lane=0;lane<4;++lane){a[lane]=next();acc[lane]=next();}
            if(index%4==1) {
                b=(b&0x807fffffu)|((96+(b%64))<<23);
                for(unsigned lane=0;lane<4;++lane) {
                    a[lane]=(a[lane]&0x807fffffu)|((96+(a[lane]%64))<<23);
                    acc[lane]=(acc[lane]&0x807fffffu)|((96+(acc[lane]%64))<<23);
                }
            } else if(index%4==2) {
                b=0x3f800000u;
                for(unsigned lane=0;lane<4;++lane) {
                    a[lane]=(a[lane]&0x807fffffu)|((25+(index%200))<<23);
                    acc[lane]=a[lane]^0x80000000u;
                    if(lane)acc[lane]^=(1u<<((index+lane)%23));
                }
            } else if(index%4==3) {
                const unsigned ea=index&255,eb=(index>>8)&255;
                b=(b&0x807fffffu)|(eb<<23);
                for(unsigned lane=0;lane<4;++lane) {
                    a[lane]=(a[lane]&0x807fffffu)|(ea<<23);
                    acc[lane]=(acc[lane]&0x807fffffu)|(((index+lane*17)&255)<<23);
                }
            }
            if(!check(a,b,acc))return false;
        }
        // Every input exponent pair, with boundary significands and signs.
        for(unsigned ea=0;ea<256;++ea)for(unsigned eb=0;eb<256;++eb) {
            a={ea<<23,(ea<<23)|0x7fffff,0x80000000u|(ea<<23),0x807fffffu|(ea<<23)};
            acc={0,0x80000000u,0x3f800000,0xbf800001};
            if(!check(a,(eb<<23)|((ea&1)?0x7fffff:0),acc))return false;
        }
        const std::array<std::uint32_t,10> zero_edge{0,0x80000000u,1,0x80000001u,0x007fffff,0x807fffff,0x00800000,0x80800000u,0x7f7fffff,0xff7fffffu};
        for(auto x:zero_edge)for(auto y:zero_edge)for(auto z:zero_edge) {
            a.fill(x);acc.fill(z);if(!check(a,y,acc))return false;
        }
    }
    std::cout<<"MADD embedded-rounding CPU/OS supported: "<<cases<<" vectors / "<<cases*4
             <<" scalar lanes; accepted="<<accepted<<" rejected="<<rejected
             <<";16 MXCSR modes, exponent grid, atomic fallback and exact flags passed\n";
    return accepted>100000&&rejected>100000;
}
