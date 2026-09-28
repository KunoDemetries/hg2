#pragma once
#include "hg/vu_matrix.hpp"
namespace hg {
struct Vu1State;
// AOT-only entry points for an instruction with a statically known XYZ mask.
// Undefined inputs retain the original checked scalar operation and fault order.
void vu_xyz_multiply_acc(Vu1State&,unsigned source,unsigned other,unsigned broadcast,bool add);
void vu_xyz_madd(Vu1State&,unsigned destination,unsigned source,unsigned other,unsigned broadcast);
}
