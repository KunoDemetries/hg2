#pragma once
#include "hg/timers.hpp"
#include "hg/intc.hpp"

namespace hg {
// EE User's Manual v6.0 p36: BUSCLK=147.456MHz. The connected diagnostic
// supplies elapsed microseconds; retain sub-cycle time instead of rounding it.
struct EeTimerClock {
    std::uint32_t fractional_cycles=0;
    void advance_us(Timers& timers,InterruptController& intc,std::uint32_t us) {
        if(us>1000000)throw std::runtime_error("EE timer clock interval exceeds one second");
        const std::uint64_t scaled=std::uint64_t(us)*147456+fractional_cycles;
        std::array<std::uint32_t,4> flags{};
        for(unsigned n=0;n<4;++n)flags[n]=timers.timers[n].mode&0xc00;
        // advance validates unsupported external/gated clocks before mutation.
        timers.advance(std::uint32_t(scaled/1000));
        fractional_cycles=std::uint32_t(scaled%1000);
        for(unsigned n=0;n<4;++n) {
            // INTC9..12 receive flag rising edges, not a level that retriggers
            // after INTC acknowledgement while the timer flag remains set.
            if(timers.timers[n].mode&~flags[n]&0xc00)intc.raise(9+n);
            timers.timers[n].interrupt=false;
        }
    }
};
}
