#include "hg/vif.hpp"
#include "hg/fpu_add4.hpp"

#if defined(_M_X64) || defined(__SSE2__)
#include <emmintrin.h>
#if defined(_MSC_VER)
#define HG_PRODUCT_INLINE __forceinline
#else
#define HG_PRODUCT_INLINE inline __attribute__((always_inline))
#endif

namespace hg {
namespace {
struct Products {
    std::array<std::uint32_t,4> bits;
    unsigned under,over;
};

HG_PRODUCT_INLINE Products broadcast_products(const std::array<std::uint32_t,4>& a,std::uint32_t b) {
    Products packed;
    if(try_fpu_product4(a,b,packed.bits,packed.under,packed.over))return packed;
    const auto zero=_mm_setzero_si128();
    const auto v=_mm_loadu_si128(reinterpret_cast<const __m128i*>(a.data()));
    const auto sign=_mm_set1_epi64x(0x80000000ull),mant=_mm_set1_epi64x(0x7fffff);
    const auto one=_mm_set1_epi64x(1);
    const auto eb=(b>>23)&255;
    if(!eb) {
        Products result{};
        _mm_storeu_si128(reinterpret_cast<__m128i*>(result.bits.data()),
            _mm_and_si128(_mm_xor_si128(v,_mm_set1_epi32(static_cast<int>(b))),
                          _mm_set1_epi32(-2147483647-1)));
        return result;
    }
    const auto exponent_bias=_mm_set1_epi64x(int(eb)-127);
    const auto bm=_mm_set1_epi64x((b&0x7fffff)|0x800000);
    const auto pair=[&](__m128i av,__m128i& under,__m128i& over) {
        // Each 64-bit lane contains one 24-bit significand. Two unsigned
        // products reproduce Fpu::product's exact 48-bit integer products.
        const auto ea=_mm_and_si128(_mm_srli_epi64(av,23),_mm_set1_epi64x(255));
        const auto product=_mm_mul_epu32(
            _mm_or_si128(_mm_and_si128(av,mant),_mm_set1_epi64x(0x800000)),bm);
        const auto wide=_mm_srli_epi64(product,47),wide_mask=_mm_sub_epi64(zero,wide);
        const auto m=_mm_or_si128(_mm_and_si128(wide_mask,_mm_srli_epi64(product,24)),
                                  _mm_andnot_si128(wide_mask,_mm_srli_epi64(product,23)));
        const auto exponent=_mm_add_epi64(_mm_add_epi64(ea,exponent_bias),wide);
        // Exponents fit signed32. Only the low32 of each64-bit lane is used;
        // active also clears the unused high halves of the comparison masks.
        const auto active=_mm_cmpgt_epi32(ea,zero);
        under=_mm_and_si128(active,_mm_cmpgt_epi32(one,exponent));
        over=_mm_and_si128(active,_mm_cmpgt_epi32(exponent,_mm_set1_epi64x(255)));
        const auto normal=_mm_andnot_si128(_mm_or_si128(under,over),active);
        auto bits=_mm_or_si128(_mm_slli_epi64(exponent,23),_mm_and_si128(m,mant));
        bits=_mm_or_si128(_mm_and_si128(normal,bits),_mm_and_si128(over,_mm_set1_epi64x(0x7fffffff)));
        return _mm_or_si128(bits,_mm_and_si128(_mm_xor_si128(av,_mm_set1_epi64x(b)),sign));
    };
    __m128i under0,over0,under1,over1;
    const auto low=pair(_mm_unpacklo_epi32(v,zero),under0,over0);
    const auto high=pair(_mm_unpackhi_epi32(v,zero),under1,over1);
    Products result{};
    _mm_storeu_si128(reinterpret_cast<__m128i*>(result.bits.data()),
        _mm_unpacklo_epi64(_mm_shuffle_epi32(low,0x88),_mm_shuffle_epi32(high,0x88)));
    const auto mask=[](__m128i value) {
        const unsigned bits=_mm_movemask_ps(_mm_castsi128_ps(value));
        return (bits&1)|((bits&4)>>1);
    };
    result.under=mask(under0)|(mask(under1)<<2);
    result.over=mask(over0)|(mask(over1)<<2);
    return result;
}

struct Result {
    std::array<std::uint32_t,4> values{};
    std::uint32_t current=0,mac=0;
};
HG_PRODUCT_INLINE bool full_result(const Vu1State& state,unsigned source,unsigned other,
                                  unsigned broadcast,bool add,Result& result) {
    // Return to the original checked path for every invalid/undefined case so
    // its exact fault order and partial-state contract remain authoritative.
    if(source>=32||other>=32||broadcast>=4||
       (source&&(state.vf_defined[source]&15)!=15)||
       (other&&!(state.vf_defined[other]&(1u<<broadcast)))||
       (add&&(state.acc_defined&15)!=15))return false;
    static constexpr std::array<std::uint32_t,4> vf_zero{0,0,0,0x3f800000u};
    unsigned under=0,over=0;
    if(!(add&&try_fpu_madd4(source?state.vf[source]:vf_zero,state.read_vf(other,broadcast),state.acc,result.values,under,over))) {
    const auto products=broadcast_products(source?state.vf[source]:vf_zero,state.read_vf(other,broadcast));
    under=products.under;over=products.over;
    unsigned add_under=0,add_over=0;
    if(add&&try_fpu_add4(state.acc,products.bits,result.values,add_under,add_over)) {
        under|=add_under;over|=add_over;
    } else for(unsigned lane=0;lane<4;++lane) {
        auto bits=products.bits[lane];
        if(add) {
            Fpu arithmetic;bits=arithmetic.add(state.acc[lane],bits);
            if(arithmetic.control&0x4000u)under|=1u<<lane;
            if(arithmetic.control&0x8000u)over|=1u<<lane;
        }
        result.values[lane]=bits;
    }
    }
    const auto values=_mm_loadu_si128(reinterpret_cast<const __m128i*>(result.values.data()));
    const auto zero_lanes=_mm_cmpeq_epi32(_mm_and_si128(values,_mm_set1_epi32(0x7fffffff)),_mm_setzero_si128());
    const unsigned zeros=_mm_movemask_ps(_mm_castsi128_ps(zero_lanes));
    const unsigned signs=unsigned(_mm_movemask_ps(_mm_castsi128_ps(values)))&~zeros;
    // MAC orders X..W from high to low within each four-bit flag group.
    static constexpr unsigned reverse[16]={0,8,4,12,2,10,6,14,1,9,5,13,3,11,7,15};
    result.mac=reverse[zeros]|(reverse[signs]<<4)|(reverse[under]<<8)|(reverse[over]<<12);
    result.current=(zeros?1u:0u)|(signs?2u:0u)|(under?4u:0u)|(over?8u:0u);
    return true;
}
}

bool try_vu_full_multiply_acc(Vu1State& state,unsigned source,unsigned other,unsigned broadcast,bool add) {
    Result result;
    if(!full_result(state,source,other,broadcast,add,result))return false;
    state.mac=result.mac;state.status=(state.status&~15u)|result.current|(result.current<<6);
    state.acc=result.values;state.acc_defined|=15;
    return true;
}
bool try_vu_full_madd_vector(Vu1State& state,unsigned destination,unsigned source,unsigned other,unsigned broadcast) {
    if(destination>=32)return false;
    Result result;
    if(!full_result(state,source,other,broadcast,true,result))return false;
    state.mac=result.mac;state.status=(state.status&~15u)|result.current|(result.current<<6);
    if(destination){state.vf[destination]=result.values;state.vf_defined[destination]|=15;}
    return true;
}
}
#undef HG_PRODUCT_INLINE
#else
namespace hg {
bool try_vu_full_multiply_acc(Vu1State&,unsigned,unsigned,unsigned,bool){return false;}
bool try_vu_full_madd_vector(Vu1State&,unsigned,unsigned,unsigned,unsigned){return false;}
}
#endif
