#pragma once
#include <cstdint>
#include <stdexcept>
#if defined(_MSC_VER) && defined(_M_X64)
#include <immintrin.h>
#endif

namespace hg {
// Exact (numerator * 2^shift) / denominator with a 64-bit quotient. Failure
// selects the existing portable checked path; no approximate result is returned.
inline bool gs_shifted_divide(std::uint64_t numerator,std::uint64_t denominator,unsigned shift,
                              std::uint64_t& quotient,std::uint64_t& remainder) {
    if(!denominator||shift>=64)return false;
    const auto high=shift?numerator>>(64-shift):0,low=numerator<<shift;
    if(high>=denominator)return false; // Native DIV must never overflow its quotient.
    if(!high){quotient=low/denominator;remainder=low%denominator;return true;}
#if defined(_MSC_VER) && defined(_M_X64)
    quotient=_udiv128(high,low,denominator,&remainder);return true;
#elif defined(__SIZEOF_INT128__)
    const auto wide=(static_cast<unsigned __int128>(high)<<64)|low;
    quotient=std::uint64_t(wide/denominator);remainder=std::uint64_t(wide%denominator);return true;
#else
    return false;
#endif
}

// Covered GS triangle weights are nonnegative and sum to an area <=65535^2.
// Their uint32 interpolant numerator is therefore <=area*UINT32_MAX. The host
// estimate is only a starting guess: integer products/remainders correct it
// in both directions, so the result is exact and independent of FP rounding.
class GsTriangleQuotient {
    std::uint64_t divisor_,maximum_;
    double reciprocal_;
public:
    explicit GsTriangleQuotient(std::uint64_t divisor):divisor_(divisor) {
        if(!divisor||divisor>65535ull*65535ull)
            throw std::runtime_error("invalid GS triangle interpolation area");
        maximum_=divisor*0xffffffffull;
        reciprocal_=1.0/double(divisor);
    }
    std::uint32_t operator()(std::uint64_t numerator) const {
        if(numerator>maximum_)
            throw std::runtime_error("GS triangle interpolant exceeds uint32 range");
        auto quotient=std::uint64_t(double(numerator)*reciprocal_);
        if(quotient>0xffffffffull)quotient=0xffffffffull;
        auto product=quotient*divisor_;
        while(product>numerator){--quotient;product-=divisor_;}
        while(numerator-product>=divisor_){++quotient;product+=divisor_;}
        return std::uint32_t(quotient);
    }
};
}
