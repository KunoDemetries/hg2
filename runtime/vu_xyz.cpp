#include "hg/vif.hpp"
#include "hg/vu_xyz.hpp"

namespace hg {
namespace {
struct Result {std::array<std::uint32_t,3> values{};unsigned mac=0,current=0;};
template<bool Add>
bool prepare(const Vu1State& state,unsigned source,unsigned other,unsigned broadcast,Result& result){
    if(source>=32||other>=32||broadcast>=4||
       (source&&(state.vf_defined[source]&7)!=7)||
       (other&&!(state.vf_defined[other]&(1u<<broadcast)))||
       (Add&&(state.acc_defined&7)!=7))return false;
    const auto scalar=other?state.vf[other][broadcast]:(broadcast==3?0x3f800000u:0u);
    for(unsigned lane=0;lane<3;++lane){
        const auto product=Fpu::product(source?state.vf[source][lane]:0u,scalar);
        auto bits=product.bits;bool under=product.underflow,over=product.overflow;
        if constexpr(Add){
            Fpu arithmetic;bits=arithmetic.add(state.acc[lane],bits);
            under=under||bool(arithmetic.control&0x4000u);
            over=over||bool(arithmetic.control&0x8000u);
        }
        result.values[lane]=bits;
        const auto flags=unsigned(!(bits&0x7fffffffu))|
            (((bits&0x80000000u)&&(bits&0x7fffffffu))?2u:0u)|(under?4u:0u)|(over?8u:0u);
        result.current|=flags;
        result.mac|=((flags&1u)|((flags&2u)<<3)|((flags&4u)<<6)|((flags&8u)<<9))<<(3-lane);
    }
    return true;
}
void commit_flags(Vu1State& state,const Result& result){
    state.mac=result.mac;state.status=(state.status&~15u)|result.current|(result.current<<6);
}
}
void vu_xyz_multiply_acc(Vu1State& state,unsigned source,unsigned other,unsigned broadcast,bool add){
    Result result;
    const bool ready=add?prepare<true>(state,source,other,broadcast,result):prepare<false>(state,source,other,broadcast,result);
    if(!ready){state.multiply_acc(source,other,14,broadcast,add);return;}
    commit_flags(state,result);
    for(unsigned lane=0;lane<3;++lane)state.acc[lane]=result.values[lane];
    state.acc_defined|=7;
}
void vu_xyz_madd(Vu1State& state,unsigned destination,unsigned source,unsigned other,unsigned broadcast){
    Result result;
    if(destination>=32||!prepare<true>(state,source,other,broadcast,result)){
        state.madd_vector(destination,source,other,14,broadcast);return;
    }
    commit_flags(state,result);
    if(destination){
        for(unsigned lane=0;lane<3;++lane)state.vf[destination][lane]=result.values[lane];
        state.vf_defined[destination]|=7;
    }
}
}
