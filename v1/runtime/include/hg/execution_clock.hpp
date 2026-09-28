#pragma once
#include <cstdint>
namespace hg {
// Experimental functional issue-slot profile, not a pipeline/cache timing model.
// EE bus147.456MHz, processor/bus ratio2; IOP36.864MHz => EE/IOP ratio8.
// Retain fractions exactly. Legacy diagnostic timing remains available for A/B.
class ExecutionClock {
    bool issue_slots_;
    std::uint32_t fraction_=0;
    unsigned iop_phase_=0;
    unsigned ee_credit_=0,iop_credit_=0;
public:
    struct Tick {std::uint32_t microseconds;bool iop_due;unsigned ee_budget=1,iop_budget=1;};
    explicit ExecutionClock(bool issue_slots):issue_slots_(issue_slots){}
    Tick step() {
        if(!issue_slots_)return {1,true};
        fraction_+=1000000;
        const auto us=fraction_/294912000;
        fraction_%=294912000;
        iop_phase_=(iop_phase_+1)%8;
        return {us,iop_phase_==0};
    }
    Tick quantum() {
        if(!issue_slots_)return {1,true};
        ee_credit_+=294912;iop_credit_+=36864;
        const Tick result{1,true,ee_credit_/1000,iop_credit_/1000};
        ee_credit_%=1000;iop_credit_%=1000;
        return result;
    }
};
}
