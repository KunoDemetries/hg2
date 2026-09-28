#include "hg/vif.hpp"
#include "hg/vu_matrix.hpp"
#include "hg/fpu_add4.hpp"
namespace hg {
// HG-DIAG-061: exact fallback and one CPU-feature dispatch per whole block.
VuTransformResult fused_vu_transform(const VuTransformSources& sources,const std::array<std::uint32_t,4>& point) {
    VuTransformResult result;
    if(try_fpu_transform(sources,point,result))return result;
    for(unsigned chain=0;chain<2;++chain) {
        std::array<std::uint32_t,4> acc{};
        for(unsigned stage=0;stage<4;++stage) {
            unsigned mac=0,current=0;
            if(chain==1&&stage==3)result.acc=acc;
            for(unsigned lane=0;lane<4;++lane) {
                const auto product=Fpu::product(sources[chain*4+stage][lane],stage==3?0x3f800000u:point[stage]);
                auto bits=product.bits;bool under=product.underflow,over=product.overflow;
                if(stage){Fpu f;bits=f.add(acc[lane],bits);under|=bool(f.control&0x4000);over|=bool(f.control&0x8000);}
                acc[lane]=bits;
                const unsigned flags=unsigned(!(bits&0x7fffffffu))|
                    (((bits&0x80000000u)&&(bits&0x7fffffffu))?2u:0u)|(under?4u:0u)|(over?8u:0u);
                current|=flags;
                mac|=((flags&1u)|((flags&2u)<<3)|((flags&4u)<<6)|((flags&8u)<<9))<<(3-lane);
            }
            result.sticky|=current;
            if(chain==1&&stage==3){result.mac=mac;result.current=current;}
        }
        result.values[chain]=acc;
    }
    return result;
}
bool prepare_vu_matrix(const Vu1State& state,const VuMatrixPlan& plan,VuMatrixResult& result) {
    if(plan.mask!=14&&plan.mask!=15)return false;
    const unsigned defined=plan.mask==14?7:15;
    for(unsigned i=0;i<4;++i) {
        const auto s=plan.sources[i],t=plan.others[i],b=plan.broadcasts[i];
        if(s>=32||t>=32||b>=4||(s&&(state.vf_defined[s]&defined)!=defined)||
           (t&&!(state.vf_defined[t]&(1u<<b))))return false;
    }
    static constexpr std::array<std::uint32_t,4> zero{0,0,0,0x3f800000u};
    std::array<std::array<std::uint32_t,4>,4> sources,values;
    std::array<std::uint32_t,4> scalars;
    std::array<unsigned,4> under,over;
    for(unsigned i=0;i<4;++i){
        sources[i]=plan.sources[i]?state.vf[plan.sources[i]]:zero;
        scalars[i]=plan.others[i]?state.vf[plan.others[i]][plan.broadcasts[i]]:zero[plan.broadcasts[i]];
    }
    if(!try_fpu_matrix4(sources,scalars,values,under,over))return false;
    for(unsigned i=0;i<4;++i){
        auto& r=result[i];r.values=values[i];r.mac=r.current=0;
        for(unsigned lane=0;lane<(plan.mask==14?3u:4u);++lane){
            const auto bits=r.values[lane];
            const unsigned flags=unsigned(!(bits&0x7fffffffu))|
                (((bits&0x80000000u)&&(bits&0x7fffffffu))?2u:0u)|
                ((under[i]&(1u<<lane))?4u:0u)|((over[i]&(1u<<lane))?8u:0u);
            r.current|=flags;
            r.mac|=((flags&1u)|((flags&2u)<<3)|((flags&4u)<<6)|((flags&8u)<<9))<<(3-lane);
        }
    }
    return true;
}
void commit_vu_matrix(Vu1State& state,const VuMatrixStage& result,unsigned mask,unsigned destination,bool accumulator){
    state.mac=result.mac;state.status=(state.status&~15u)|result.current|(result.current<<6);
    const unsigned defined=mask==14?7:15;
    if(accumulator){
        for(unsigned lane=0;lane<(mask==14?3u:4u);++lane)state.acc[lane]=result.values[lane];
        state.acc_defined|=defined;
    } else if(destination){
        for(unsigned lane=0;lane<(mask==14?3u:4u);++lane)state.vf[destination][lane]=result.values[lane];
        state.vf_defined[destination]|=defined;
    }
}
}
