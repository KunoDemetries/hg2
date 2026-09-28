#pragma once
#include <array>
#include <cstdint>
#include <stdexcept>

namespace hg {
struct Timers {
    struct Timer { std::uint32_t count=0, mode=0, compare=0, hold=0, phase=0; bool interrupt=false; };
    std::array<Timer,4> timers{};
    bool clock_attached=false;
    static bool contains(std::uint32_t a) { return a>=0x10000000u && a<0x10002000u; }
    std::uint32_t read(std::uint32_t a) {
        auto& t=timers.at((a-0x10000000u)/0x800);
        switch(a&0x7ff) {
        case 0:
            if((t.mode&0x80) && !clock_attached) throw std::runtime_error("timer count needs a host clock scheduler");
            return t.count;
        case 0x10:return t.mode;
        case 0x20:return t.compare;
        case 0x30:if(a<0x10001000) return t.hold; break;
        }
        throw std::runtime_error("unimplemented timer register");
    }
    void write(std::uint32_t a,std::uint32_t value) {
        auto& t=timers.at((a-0x10000000u)/0x800);
        switch(a&0x7ff) {
        case 0:t.count=value&65535; t.phase=0; return;
        case 0x10:
            t.mode=(t.mode&0xc00&~value)|(value&0x3ff);
            return;
        case 0x20:t.compare=value&65535; return;
        case 0x30:if(a<0x10001000) {t.hold=value&65535;return;} break;
        }
        throw std::runtime_error("unimplemented timer register write");
    }
    void advance(std::uint32_t bus_cycles) {
        for(const auto& t:timers)
            if((t.mode&0x80) && ((t.mode&4) || (t.mode&3)==3))
                throw std::runtime_error("gated/external timer clock not implemented");
        clock_attached=true;
        for(auto& t:timers) {
            if(!(t.mode&0x80)) continue;
            const unsigned divisor=(t.mode&3)==0 ? 1 : (t.mode&3)==1 ? 16 : 256;
            const std::uint64_t cycles=std::uint64_t(t.phase)+bus_cycles;
            auto ticks=cycles/divisor; t.phase=unsigned(cycles%divisor);
            while(ticks--) {
                ++t.count;
                if(t.count==0x10000) {
                    t.count=0;
                    if(t.mode&0x200) { t.mode|=0x800; t.interrupt=true; }
                }
                if(t.count==t.compare) {
                    if(t.mode&0x100) { t.mode|=0x400; t.interrupt=true; }
                    if(t.mode&0x40) t.count=0;
                }
            }
        }
    }
};
}
