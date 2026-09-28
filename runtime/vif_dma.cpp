// HG-FAIL-032: experimental reference fixture; compiled into runtime_tests only.
// No gameplay activation is retained without a repeatable qualified FPS benefit.
#include "hg/vif_dma.hpp"
#include "hg/runtime.hpp"
#include <algorithm>

namespace hg {
std::uint32_t buffer_incomplete_vif_direct(State& s) {
    auto& channel=s.dmac.channels[1];
    if(!s.dmac.enabled(1)||!(channel.chcr&0x100u)||channel.qwc<3)return 0;
    const auto mode=channel.chcr&0xcu;
    if(mode!=0&&(mode!=4||!s.dmac.vif1_packet))return 0;

    auto& v=s.vif1;
    const auto have=v.pending.size();
    if(have<4||have>Vif1Path::max_pending||(v.pending_phase+4)%16)return 0;
    const auto code=Vif1Path::word(v.pending,0);
    const auto command=code>>24;
    if(command!=0x50u&&command!=0x51u)return 0; // Includes interrupt-bit rejection.
    const auto units=std::uint16_t(code)?std::size_t(std::uint16_t(code)):std::size_t(65536);
    const auto need=4+units*16;
    if(have>=need)return 0;

    const bool scratch=(channel.madr&0x80000000u)!=0;
    const auto offset=std::size_t(channel.madr&0x7fffffffu);
    const auto size=scratch?s.scratch.size():s.ram.size();
    if(offset%16||offset>=size)return 0; // Original pump reports invalid sources.
    // The -1 bounds leave the completing input word and DMA completion to the
    // original pump, including its exact partial-state and error ordering.
    const auto count=std::min({std::size_t(channel.qwc-1),(need-have-1)/16,(size-offset)/16});
    if(count<2)return 0;
    const auto* source=(scratch?s.scratch.data():s.ram.data())+offset;
    v.pending.insert(v.pending.end(),source,source+count*16);
    channel.madr+=static_cast<std::uint32_t>(count*16);
    channel.qwc-=static_cast<std::uint32_t>(count);
    // With no completed command, GS is unchanged. Repeated original sync calls
    // only OR the same latched INTC bit; no interrupt service interleaves here.
    s.sync_gs_interrupt();
    return static_cast<std::uint32_t>(count);
}
}
