#pragma once
#include "hg/iop.hpp"

namespace hg {
// Native service gateway in the reserved host-service address range. The
// original SIFCMD dispatcher invokes it after receiving a real DMA packet.
struct IopRebootRequest {
    static constexpr std::uint32_t gateway=0x1f0030;
    bool pending=false;
    std::uint32_t mode=0;
    std::string arguments;
    void receive(IopState& s) {
        const auto address=s.r(4);
        if(pending || !s.in_interrupt || address%4 || std::uint64_t(address)+104>s.ram.size())
            throw IopFault(s.pc,"invalid native IOP reboot callback context");
        // Layout independently decoded from EE 0x275700: 16-byte header,
        // argument byte count, mode, and an 80-byte argument buffer.
        // SIFCMD 0x98740 clears the size byte before copying to its callback
        // stack buffer. The callback therefore receives a consumed header.
        if(s.load(address,4,false)!=0 || s.load(address+8,4,false)!=0x80000003)
            throw IopFault(s.pc,"invalid IOP reboot command packet");
        const auto length=s.load(address+16,4,false);
        if(length>79)throw IopFault(s.pc,"IOP reboot arguments exceed packet capacity");
        std::string text;
        for(std::uint32_t n=0;n<length;++n) {
            const auto byte=s.load(address+24+n,1,false);
            if(!byte)throw IopFault(s.pc,"embedded terminator in IOP reboot arguments");
            text.push_back(static_cast<char>(byte));
        }
        mode=s.load(address+20,4,false);arguments=std::move(text);pending=true;
        const auto target=s.r(31);s.finish_instruction();s.pc=target;
    }
};
}
