#pragma once
#include "hg/ipu_dct_tables.hpp"
#include <array>
#include <cstdint>

namespace hg {
// Index the existing H.262 codebooks by their longest (16-bit) prefix.
// Entries are one-based byte indices; zero retains the explicit invalid-code
// result. Fill in source order to preserve the linear lookup's first match.
template<std::size_t N>
std::array<std::uint8_t,65536> make_ipu_dct_index(const IpuDctCode (&table)[N]) {
    static_assert(N<256);
    std::array<std::uint8_t,65536> result{};
    for(unsigned n=0;n<N;++n) {
        const auto& entry=table[n];
        const unsigned first=entry.code<<(16-entry.bits),count=1u<<(16-entry.bits);
        for(unsigned p=first;p<first+count;++p)if(!result[p])result[p]=std::uint8_t(n+1);
    }
    return result;
}
inline const IpuDctCode* lookup_ipu_dct(bool table_one,std::uint32_t prefix) {
    static const auto zero=make_ipu_dct_index(ipu_dct_0);
    static const auto one=make_ipu_dct_index(ipu_dct_1);
    const auto index=(table_one?one:zero)[prefix>>16];
    return index?&(table_one?ipu_dct_1:ipu_dct_0)[index-1]:nullptr;
}
}
