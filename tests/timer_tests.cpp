#include "hg/runtime.hpp"
#include "hg/ee_timer_clock.hpp"
#include "hg/execution_clock.hpp"
#include <iostream>
#define CHECK(x) do { if (!(x)) { std::cerr << "Failed line " << __LINE__ << ": " << #x << '\n'; return 1; } } while (0)
int main() {
    {
        hg::ExecutionClock clock(true),legacy(false);
        unsigned us=0,iop_slots=0;
        for(unsigned n=0;n<294912;++n) {
            const auto tick=clock.step();CHECK(tick.microseconds<=1);
            us+=tick.microseconds;iop_slots+=tick.iop_due;
        }
        CHECK(us==1000 && iop_slots==36864);
        hg::ExecutionClock burst(true);unsigned ee_budget=0,iop_budget=0;
        for(unsigned n=0;n<1000;++n) {
            const auto tick=burst.quantum();CHECK(tick.microseconds==1);
            ee_budget+=tick.ee_budget;iop_budget+=tick.iop_budget;
        }
        CHECK(ee_budget==294912 && iop_budget==36864);
        for(unsigned n=0;n<1000;++n) {
            const auto tick=legacy.step();CHECK(tick.microseconds==1 && tick.iop_due);
        }
    }
    hg::State s;
    CHECK(s.load(0x10001810,4,false)==0);
    s.store(0x10001820,4,3); s.store(0x10001810,4,0x1c1);
    bool fault=false;
    try {s.load(0x10001800,4,false);} catch(const hg::Fault&) {fault=true;}
    CHECK(fault);
    s.timers.advance(47); CHECK(s.load(0x10001800,4,false)==2);
    s.timers.advance(1); CHECK(s.load(0x10001800,4,false)==0);
    CHECK(s.load(0x10001810,4,false)==0x5c1 && s.timers.timers[3].interrupt);
    s.store(0x10001810,4,0x5c1); CHECK(s.load(0x10001810,4,false)==0x1c1);
    s.store(0x10001800,4,0xffff); s.store(0x10001810,4,0x280);
    s.timers.advance(1); CHECK(s.load(0x10001810,4,false)==0xa80);
    s.store(0x10001810,4,0x284); fault=false;
    try {s.timers.advance(1);} catch(const std::runtime_error&) {fault=true;}
    CHECK(fault);
    hg::Timers connected;hg::InterruptController intc;hg::EeTimerClock clock;
    connected.write(0x10000810,0x82); // Observed original timer1, BUSCLK/256.
    for(unsigned n=0;n<1000;++n)clock.advance_us(connected,intc,1);
    CHECK(connected.read(0x10000800)==576 && clock.fractional_cycles==0);
    CHECK(intc.status==0);
    connected.write(0x10000800,65535);
    clock.advance_us(connected,intc,1);CHECK(connected.read(0x10000800)==65535);
    clock.advance_us(connected,intc,1);CHECK(connected.read(0x10000800)==0);
    connected.write(0x10000810,0);const auto held=connected.read(0x10000800);
    clock.advance_us(connected,intc,1000);CHECK(connected.read(0x10000800)==held);
    connected.write(0x10000800,0);connected.write(0x10000820,2);
    connected.write(0x10000810,0x1c2); // compare/reset every two ticks
    clock.advance_us(connected,intc,10);CHECK(intc.status==(1u<<10));
    intc.write(0x1000f000,1u<<10);
    clock.advance_us(connected,intc,10);CHECK(intc.status==0); // flag still latched
    connected.write(0x10000810,0x5c2);
    clock.advance_us(connected,intc,10);CHECK(intc.status==(1u<<10));
    intc.write(0x1000f000,1u<<10);
    connected.write(0x10000800,65535);connected.write(0x10000810,0x682);
    clock.advance_us(connected,intc,2);CHECK(intc.status==(1u<<10));
    connected.write(0x10000810,0x84);
    const auto fraction=clock.fractional_cycles,count=connected.timers[1].count;
    fault=false;try{clock.advance_us(connected,intc,1);}catch(const std::runtime_error&){fault=true;}
    CHECK(fault && clock.fractional_cycles==fraction && connected.timers[1].count==count);
    return 0;
}
