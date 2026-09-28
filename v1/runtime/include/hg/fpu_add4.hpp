#pragma once
#include <array>
#include <cstdint>
namespace hg {
bool try_fpu_add4(const std::array<std::uint32_t,4>& a,const std::array<std::uint32_t,4>& b,
                  std::array<std::uint32_t,4>& result,unsigned& under,unsigned& over);
bool try_fpu_product4(const std::array<std::uint32_t,4>& a,std::uint32_t b,
                     std::array<std::uint32_t,4>& result,unsigned& under,unsigned& over);
// Guarded embedded-rounding candidate. Rejection changes neither output nor flags.
bool fpu_madd4_er_available() noexcept;
bool try_fpu_madd4_er(const std::array<std::uint32_t,4>& a,std::uint32_t b,
                      const std::array<std::uint32_t,4>& acc,std::array<std::uint32_t,4>& result,
                      unsigned& under,unsigned& over) noexcept;
// Exact product followed by exact addition; this is not a fused guest operation.
bool try_fpu_madd4(const std::array<std::uint32_t,4>& a,std::uint32_t b,const std::array<std::uint32_t,4>& acc,
                   std::array<std::uint32_t,4>& result,unsigned& under,unsigned& over);

bool try_fpu_matrix4(const std::array<std::array<std::uint32_t,4>,4>& sources,
                     const std::array<std::uint32_t,4>& scalars,
                     std::array<std::array<std::uint32_t,4>,4>& values,
                     std::array<unsigned,4>& under,std::array<unsigned,4>& over);
}
