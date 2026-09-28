#pragma once
#include <array>
#include <cstdint>
namespace hg {
struct Vu1State;
// HG-DIAG-061: live outputs of a statically proved two-transform block.
struct VuTransformResult {
    std::array<std::array<std::uint32_t,4>,2> values;
    std::array<std::uint32_t,4> acc;
    unsigned mac=0,current=0,sticky=0;
};
using VuTransformSources=std::array<const std::uint32_t*,8>;
VuTransformResult fused_vu_transform(const VuTransformSources&,const std::array<std::uint32_t,4>&);
struct VuMatrixPlan {
    std::array<unsigned,4> sources,others,broadcasts;
    unsigned mask;
};
struct VuMatrixStage {
    std::array<std::uint32_t,4> values;
    unsigned mac,current;
};
using VuMatrixResult=std::array<VuMatrixStage,4>;
// Pure preparation for a statically proven, uninterrupted accumulator chain.
// False leaves state untouched; original operations retain fault ordering.
bool prepare_vu_matrix(const Vu1State&,const VuMatrixPlan&,VuMatrixResult&);
void commit_vu_matrix(Vu1State&,const VuMatrixStage&,unsigned mask,unsigned destination,bool accumulator);
}
