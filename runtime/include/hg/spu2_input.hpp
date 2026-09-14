#pragma once
#include <array>
#include <cstdint>
#include <stdexcept>

namespace hg {
// SPU2 Overview pp10,13-14,28: 48 kHz stereo input, 512 short words
// per channel, double buffered in halves of 256 samples. This is the input
// transport, not voice synthesis, volume processing, effects, or final mixing.
struct Spu2Input {
    static constexpr unsigned samples_per_half=256,bytes_per_half=1024;
    struct Core {
        bool enabled=false,started=false;
        std::array<bool,2> ready{};
        unsigned read_frame=0,write_half=0;
        std::uint64_t phase=0,frames=0;
    };
    std::array<Core,2> cores{};
    static std::uint32_t sample_address(unsigned core,unsigned channel,unsigned frame) {
        if(core>=2 || channel>=2 || frame>=512)throw std::runtime_error("invalid SPU2 input sample address");
        return 2u*(0x2000u+core*0x400u+channel*0x200u+frame);
    }
    void enable(unsigned core,bool enabled) {
        auto& c=cores.at(core);
        if(c.enabled==enabled)return;
        c={};c.enabled=enabled;
    }
    bool can_fill(unsigned core) const {
        const auto& c=cores.at(core);return c.enabled && !c.ready[c.write_half];
    }
    // The supplied CRI input queues alternate 512-byte channel blocks.
    // Callers supply a fully bounded 1024-byte L/R pair. Publish readiness only
    // after both channels have been copied into their separate local areas.
    template<class ReadByte,class WriteByte>
    void fill(unsigned core,ReadByte read,WriteByte write) {
        auto& c=cores.at(core);
        if(!can_fill(core))throw std::runtime_error("SPU2 input half is not available");
        for(unsigned channel=0;channel<2;++channel) {
            const auto destination=sample_address(core,channel,c.write_half*samples_per_half);
            for(unsigned n=0;n<512;++n)write(destination+n,read(channel*512+n));
        }
        c.ready[c.write_half]=true;c.write_half^=1;c.started=true;
    }
    template<class ReadShort,class Emit,class Refill>
    void advance(std::uint32_t microseconds,ReadShort read,Emit emit,Refill refill) {
        for(unsigned core=0;core<2;++core) {
            auto& c=cores[core];if(!c.enabled || !c.started)continue;
            c.phase+=std::uint64_t(microseconds)*48000u;
            while(c.phase>=1000000u) {
                const auto half=c.read_frame/samples_per_half;
                if(!c.ready[half])throw std::runtime_error("SPU2 input underrun requires validated hardware behavior");
                const auto left=read(sample_address(core,0,c.read_frame));
                const auto right=read(sample_address(core,1,c.read_frame));
                emit(core,left,right);
                c.phase-=1000000u;++c.frames;
                c.read_frame=(c.read_frame+1)%512;
                if(c.read_frame%samples_per_half==0) {c.ready[half]=false;refill(core);}
            }
        }
    }
};
}
