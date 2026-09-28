#pragma once
#include "hg/vif.hpp"

#if defined(_MSC_VER)
#define HG_VU_SPECIALIZED_NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define HG_VU_SPECIALIZED_NOINLINE __attribute__((noinline))
#else
#define HG_VU_SPECIALIZED_NOINLINE
#endif

namespace hg {
// AOT-only constant operands; no runtime instruction decoding or dispatch.
// Keep the helper outside the generated CFG. Its operation/fault sequence is
// deliberately the same as the checked scalar multiply_vector reference.
template<unsigned Destination, unsigned Source, unsigned Other,
         unsigned Mask, unsigned Broadcast>
HG_VU_SPECIALIZED_NOINLINE void vu_aot_multiply(Vu1State& state) {
    std::array<std::uint32_t,4> values{};
    std::array<bool,4> underflow{},overflow{};
    // The broadcast read is observable even when no destination lane is active.
    const auto scalar=state.read_vf(Other,Broadcast);
    for(unsigned lane=0;lane!=4;++lane)if(Mask&(8u>>lane)) {
        const auto product=Fpu::product(state.read_vf(Source,lane),scalar);
        values[lane]=product.bits;
        underflow[lane]=product.underflow;
        overflow[lane]=product.overflow;
    }
    // All source reads precede flag publication, but an invalid destination
    // faults after these flags change, exactly as in the reference method.
    state.update_arithmetic_flags(values,Mask,underflow,overflow);
    for(unsigned lane=0;lane!=4;++lane)if(Mask&(8u>>lane))
        state.write_vf(Destination,lane,values[lane]);
}

// Compact gameplay candidate: keep only the hot mask/broadcast operands static.
// Register indices remain ordinary runtime operands, avoiding one template body
// for every AOT instruction while preserving multiply_vector's operation order.
template<unsigned Mask, unsigned Broadcast>
HG_VU_SPECIALIZED_NOINLINE void vu_masked_multiply(
        Vu1State& state,unsigned destination,unsigned source,unsigned other) {
    static_assert(Mask==14||Mask==15);
    static_assert(Broadcast<4);
    std::array<std::uint32_t,4> values{};
    std::array<bool,4> underflow{},overflow{};
    const auto scalar=state.read_vf(other,Broadcast);
    for(unsigned lane=0;lane!=4;++lane)if(Mask&(8u>>lane)) {
        const auto product=Fpu::product(state.read_vf(source,lane),scalar);
        values[lane]=product.bits;
        underflow[lane]=product.underflow;
        overflow[lane]=product.overflow;
    }
    state.update_arithmetic_flags(values,Mask,underflow,overflow);
    for(unsigned lane=0;lane!=4;++lane)if(Mask&(8u>>lane))
        state.write_vf(destination,lane,values[lane]);
}
} // namespace hg

#undef HG_VU_SPECIALIZED_NOINLINE
