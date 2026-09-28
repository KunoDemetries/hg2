#pragma once
#include <array>
#include <cstdint>
namespace hg {
struct Vu1State;
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
