#include <cstdint>
#if defined(_M_X64) || defined(__x86_64__)
#include <immintrin.h>
#endif

namespace hg {
#if (defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__)
__attribute__((target("avx512f,avx512dq,avx512bw,avx512vl,avx512cd")))
#endif
bool fpu_madd4_er_impl(const std::uint32_t* source,std::uint32_t scalar,
                       const std::uint32_t* accumulator,std::uint32_t* result,
                       unsigned& under,unsigned& over) noexcept {
#if defined(_M_X64) || defined(__x86_64__)
    // The caller checks CPU and OS state before entering this isolated ISA unit.
    constexpr __mmask16 lanes=15;
    constexpr int rounding=_MM_FROUND_TO_ZERO|_MM_FROUND_NO_EXC;
    const auto zero=_mm512_setzero_si512();
    const auto sign=_mm512_set1_epi32(-2147483647-1);
    const auto magnitude=_mm512_set1_epi32(0x7fffffff);
    const auto exponent_mask=_mm512_set1_epi32(255);
    auto a=_mm512_maskz_loadu_epi32(lanes,source);
    auto b=_mm512_set1_epi32(static_cast<int>(scalar));
    auto acc=_mm512_maskz_loadu_epi32(lanes,accumulator);
    const auto ea=_mm512_and_si512(_mm512_srli_epi32(a,23),exponent_mask);
    const auto eb=_mm512_and_si512(_mm512_srli_epi32(b,23),exponent_mask);
    const auto ec=_mm512_and_si512(_mm512_srli_epi32(acc,23),exponent_mask);
    // Guest exponent255 is extended finite, not host NaN/infinity.
    if(lanes&(_mm512_cmpeq_epi32_mask(ea,exponent_mask)|
              _mm512_cmpeq_epi32_mask(eb,exponent_mask)|
              _mm512_cmpeq_epi32_mask(ec,exponent_mask)))return false;
    const auto az=_mm512_cmpeq_epi32_mask(ea,zero);
    const auto bz=_mm512_cmpeq_epi32_mask(eb,zero);
    const auto cz=_mm512_cmpeq_epi32_mask(ec,zero);
    const auto sum=_mm512_add_epi32(ea,eb);
    const auto product_bad=_mm512_cmplt_epi32_mask(sum,_mm512_set1_epi32(128))|
                           _mm512_cmpgt_epi32_mask(sum,_mm512_set1_epi32(380));
    if(lanes&~(az|bz)&product_bad)return false;
    // All exponent-zero encodings denote signed zero in the existing model.
    a=_mm512_mask_mov_epi32(a,az,_mm512_and_si512(a,sign));
    b=_mm512_mask_mov_epi32(b,bz,_mm512_and_si512(b,sign));
    acc=_mm512_mask_mov_epi32(acc,cz,_mm512_and_si512(acc,sign));
    const auto product=_mm512_castps_si512(_mm512_mul_round_ps(
        _mm512_castsi512_ps(a),_mm512_castsi512_ps(b),rounding));
    // product exponent is1..254 or exact signed zero; no IEEE boundary can
    // silently saturate/flush differently from the guest arithmetic profile.
    const auto pm=_mm512_and_si512(product,magnitude);
    const auto cm=_mm512_and_si512(acc,magnitude);
    const auto swap=_mm512_cmpgt_epu32_mask(pm,cm);
    const auto big=_mm512_mask_blend_epi32(swap,acc,product);
    auto small=_mm512_mask_blend_epi32(swap,product,acc);
    const auto be=_mm512_and_si512(_mm512_srli_epi32(big,23),exponent_mask);
    const auto se=_mm512_and_si512(_mm512_srli_epi32(small,23),exponent_mask);
    const auto add_bad=_mm512_cmplt_epi32_mask(be,_mm512_set1_epi32(24))|
                       _mm512_cmpgt_epi32_mask(be,_mm512_set1_epi32(253));
    if(lanes&add_bad&~_mm512_cmpeq_epi32_mask(be,zero))return false;
    const auto delta=_mm512_sub_epi32(be,se);
    // Guest addition discards the small operand's low significand bits BEFORE
    // subtraction/addition. Host round-to-zero alone would not reproduce this.
    const auto keep=_mm512_sllv_epi32(_mm512_set1_epi32(-1),delta);
    const auto chopped=_mm512_and_si512(small,keep);
    small=_mm512_mask_blend_epi32(_mm512_cmpgt_epi32_mask(delta,_mm512_set1_epi32(23)),
                                  chopped,_mm512_and_si512(small,sign));
    // For be>=24, every nonzero aligned difference has normal exponent>=1.
    // For be<=253, a sum cannot exceed exponent254. Exact cancellation and
    // signed-zero addition have the same RTZ sign rules as the integer model.
    const auto added=_mm512_add_round_ps(_mm512_castsi512_ps(big),
                                         _mm512_castsi512_ps(small),rounding);
    _mm512_mask_storeu_epi32(result,lanes,_mm512_castps_si512(added));
    under=over=0;
    return true;
#else
    (void)source;(void)scalar;(void)accumulator;(void)result;(void)under;(void)over;
    return false;
#endif
}
}
