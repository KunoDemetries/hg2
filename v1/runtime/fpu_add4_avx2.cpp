#include "hg/fpu.hpp"
#include <array>
#if defined(_M_X64) || defined(__x86_64__)
#include <immintrin.h>
#endif
namespace hg {
#if defined(_MSC_VER)
#define HG_AVX_INLINE __forceinline
#else
#define HG_AVX_INLINE inline __attribute__((always_inline))
#endif
namespace {

#if (defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__)
__attribute__((target("avx2")))
#endif
HG_AVX_INLINE void product4_impl(const std::uint32_t* a,std::uint32_t b,std::uint32_t* result,
                      unsigned& under,unsigned& over) {
#if defined(_M_X64) || defined(__x86_64__)
    const auto zero=_mm256_setzero_si256();
    const auto av=_mm256_cvtepu32_epi64(_mm_loadu_si128(reinterpret_cast<const __m128i*>(a)));
    const auto sign=_mm256_and_si256(_mm256_xor_si256(av,_mm256_set1_epi64x(b)),
                                    _mm256_set1_epi64x(0x80000000ull));
    const unsigned eb=(b>>23)&255;
    __m256i bits=sign;under=over=0;
    if(eb) {
        const auto fraction=_mm256_set1_epi64x(0x7fffff);
        const auto ea=_mm256_and_si256(_mm256_srli_epi64(av,23),_mm256_set1_epi64x(255));
        const auto product=_mm256_mul_epu32(
            _mm256_or_si256(_mm256_and_si256(av,fraction),_mm256_set1_epi64x(0x800000)),
            _mm256_set1_epi64x((b&0x7fffff)|0x800000));
        const auto wide=_mm256_srli_epi64(product,47),wide_mask=_mm256_sub_epi64(zero,wide);
        const auto mantissa=_mm256_or_si256(_mm256_and_si256(wide_mask,_mm256_srli_epi64(product,24)),
                                           _mm256_andnot_si256(wide_mask,_mm256_srli_epi64(product,23)));
        const auto exponent=_mm256_add_epi64(_mm256_add_epi64(ea,_mm256_set1_epi64x(int(eb)-127)),wide);
        const auto active=_mm256_cmpgt_epi64(ea,zero);
        const auto uf=_mm256_and_si256(active,_mm256_cmpgt_epi64(_mm256_set1_epi64x(1),exponent));
        const auto of=_mm256_and_si256(active,_mm256_cmpgt_epi64(exponent,_mm256_set1_epi64x(255)));
        const auto normal=_mm256_andnot_si256(_mm256_or_si256(uf,of),active);
        const auto magnitude=_mm256_or_si256(_mm256_slli_epi64(exponent,23),_mm256_and_si256(mantissa,fraction));
        bits=_mm256_or_si256(sign,_mm256_or_si256(_mm256_and_si256(normal,magnitude),
                                                 _mm256_and_si256(of,_mm256_set1_epi64x(0x7fffffff))));
        under=unsigned(_mm256_movemask_pd(_mm256_castsi256_pd(uf)));
        over=unsigned(_mm256_movemask_pd(_mm256_castsi256_pd(of)));
    }
    const auto packed=_mm256_permutevar8x32_epi32(bits,_mm256_setr_epi32(0,2,4,6,0,0,0,0));
    _mm_storeu_si128(reinterpret_cast<__m128i*>(result),_mm256_castsi256_si128(packed));
#else
    under=over=0;
    for(unsigned lane=0;lane<4;++lane) {
        const auto p=Fpu::product(a[lane],b);result[lane]=p.bits;
        if(p.underflow)under|=1u<<lane;if(p.overflow)over|=1u<<lane;
    }
#endif
}
#if (defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__)
__attribute__((target("avx2")))
#endif
HG_AVX_INLINE void add4_impl(const std::uint32_t* a,const std::uint32_t* b,std::uint32_t* result,
                   unsigned& under,unsigned& over) {
#if defined(_M_X64) || defined(__x86_64__)
    const auto zero=_mm_setzero_si128(),sign=_mm_set1_epi32(-2147483647-1);
    const auto fraction=_mm_set1_epi32(0x7fffff),hidden=_mm_set1_epi32(0x800000);
    auto av=_mm_loadu_si128(reinterpret_cast<const __m128i*>(a));
    auto bv=_mm_loadu_si128(reinterpret_cast<const __m128i*>(b));
    auto ea=_mm_and_si128(_mm_srli_epi32(av,23),_mm_set1_epi32(255));
    auto eb=_mm_and_si128(_mm_srli_epi32(bv,23),_mm_set1_epi32(255));
    const auto az=_mm_cmpeq_epi32(ea,zero),bz=_mm_cmpeq_epi32(eb,zero);
    av=_mm_andnot_si128(_mm_andnot_si128(sign,az),av);
    bv=_mm_andnot_si128(_mm_andnot_si128(sign,bz),bv);
    const auto both_zero=_mm_and_si128(az,bz);
    auto special=_mm_or_si128(_mm_and_si128(az,bv),_mm_and_si128(bz,av));
    special=_mm_blendv_epi8(special,_mm_and_si128(av,bv),both_zero);
    const auto swap=_mm_cmpgt_epi32(_mm_andnot_si128(sign,bv),_mm_andnot_si128(sign,av));
    const auto old_a=av,old_ea=ea;
    av=_mm_blendv_epi8(av,bv,swap);bv=_mm_blendv_epi8(bv,old_a,swap);
    ea=_mm_blendv_epi8(ea,eb,swap);eb=_mm_blendv_epi8(eb,old_ea,swap);
    const auto ma=_mm_or_si128(_mm_and_si128(av,fraction),hidden);
    const auto mb=_mm_srlv_epi32(_mm_or_si128(_mm_and_si128(bv,fraction),hidden),_mm_sub_epi32(ea,eb));
    auto m=_mm_blendv_epi8(_mm_add_epi32(ma,mb),_mm_sub_epi32(ma,mb),_mm_srai_epi32(_mm_xor_si128(av,bv),31));
    const auto nonzero=_mm_cmpgt_epi32(m,zero);
    const auto wide=_mm_srli_epi32(m,24);m=_mm_srlv_epi32(m,wide);ea=_mm_add_epi32(ea,wide);
    // After the carry shift, 0 <= m <= 2^24-1. Conversion to binary32 is
    // therefore exact in every host rounding mode and raises no FP exception.
    // Its exponent supplies an exact bit length, not a rounded guest sum.
    const auto exponent=_mm_srli_epi32(_mm_castps_si128(_mm_cvtepi32_ps(m)),23);
    const auto amount=_mm_sub_epi32(_mm_set1_epi32(150),exponent);
    m=_mm_sllv_epi32(m,amount);ea=_mm_sub_epi32(ea,amount);
    const auto active=_mm_andnot_si128(_mm_or_si128(az,bz),nonzero);
    const auto uf=_mm_and_si128(active,_mm_cmpgt_epi32(_mm_set1_epi32(1),ea));
    const auto of=_mm_and_si128(active,_mm_cmpgt_epi32(ea,_mm_set1_epi32(255)));
    auto bits=_mm_or_si128(_mm_slli_epi32(ea,23),_mm_and_si128(m,fraction));
    bits=_mm_blendv_epi8(bits,_mm_set1_epi32(0x7fffffff),of);
    bits=_mm_andnot_si128(uf,bits);
    bits=_mm_and_si128(nonzero,_mm_or_si128(bits,_mm_and_si128(av,sign)));
    bits=_mm_blendv_epi8(bits,special,_mm_or_si128(az,bz));
    _mm_storeu_si128(reinterpret_cast<__m128i*>(result),bits);
    under=unsigned(_mm_movemask_ps(_mm_castsi128_ps(uf)));
    over=unsigned(_mm_movemask_ps(_mm_castsi128_ps(of)));
#else
    under=over=0;
    for(unsigned lane=0;lane<4;++lane){Fpu f;result[lane]=f.add(a[lane],b[lane]);
        if(f.control&0x4000)under|=1u<<lane;if(f.control&0x8000)over|=1u<<lane;}
#endif
}
} // anonymous namespace
#if (defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__)
__attribute__((target("avx2")))
#endif
void fpu_product4_avx2(const std::uint32_t* a,std::uint32_t b,std::uint32_t* result,unsigned& under,unsigned& over) {
    product4_impl(a,b,result,under,over);
}
#if (defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__)
__attribute__((target("avx2")))
#endif
void fpu_add4_avx2(const std::uint32_t* a,const std::uint32_t* b,std::uint32_t* result,unsigned& under,unsigned& over) {
    add4_impl(a,b,result,under,over);
}
#if (defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__)
__attribute__((target("avx2")))
#endif
void fpu_madd4_avx2(const std::uint32_t* a,std::uint32_t b,const std::uint32_t* acc,std::uint32_t* result,unsigned& under,unsigned& over) {
    std::uint32_t products[4];unsigned product_under,product_over;
    product4_impl(a,b,products,product_under,product_over);
    add4_impl(acc,products,result,under,over);
    under|=product_under;over|=product_over;
}
#if (defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__)
__attribute__((target("avx2")))
#endif
void fpu_matrix4_avx2(const std::array<std::array<std::uint32_t,4>,4>& sources,
                     const std::array<std::uint32_t,4>& scalars,
                     std::array<std::array<std::uint32_t,4>,4>& values,
                     std::array<unsigned,4>& under,std::array<unsigned,4>& over) {
    std::array<std::array<std::uint32_t,4>,4> products;
    for(unsigned i=0;i<4;++i)product4_impl(sources[i].data(),scalars[i],products[i].data(),under[i],over[i]);
    values[0]=products[0];
    for(unsigned i=1;i<4;++i){unsigned u,o;
        add4_impl(values[i-1].data(),products[i].data(),values[i].data(),u,o);
        under[i]|=u;over[i]|=o;
    }
}
#undef HG_AVX_INLINE
}
