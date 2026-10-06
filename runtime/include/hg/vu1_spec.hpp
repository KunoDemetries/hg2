#pragma once
#include <cstdint>

namespace hg {
struct Vif1Path;
struct Vu1UnpackLog;
// Static inputs of one AOT VU1 activation, generated from the configured
// microprogram graphs (tools/hgtool/vu_emit.py _spec_live_in): VF lanes some path
// may read before writing them (bit reg*4+lane, lane 0=X; vf[0] holds VF0-VF15)
// and misc bits (VI1-VI15 at their index, ACC X..W at 16..19, I at 20). Q/P
// inputs are detected at runtime. known=false for unknown programs/entries.
struct Vu1SpecInfo {
    std::uint64_t vf_low=0,vf_high=0;
    std::uint32_t misc=0;
    bool known=false;
    bool vf_live(unsigned reg,unsigned lane) const {
        const auto bit=reg*4+lane;
        return ((bit<64?vf_low>>bit:vf_high>>(bit-64))&1)!=0;
    }
};
// Misc bits shared by Vu1SpecInfo::misc and Vu1State::spec_written_misc. The
// written mask also marks an executed DIV/SQRT/RSQRT (Q group) or EFU (P group).
constexpr unsigned vu1_spec_acc_bit=16,vu1_spec_i_bit=20,vu1_spec_q_bit=21,vu1_spec_p_bit=22;
using Vu1SpecInfoQuery=Vu1SpecInfo(*)(const Vif1Path&,std::uint16_t entry);
// Set by the generated image configuration; null without compiled VU1 programs.
extern Vu1SpecInfoQuery vu1_spec_info_query;
// Optional log of VU1 data-memory vectors written by VIF1 UNPACK on this thread.
// Null by default; recording has no guest-state effect.
extern thread_local Vu1UnpackLog* vif1_unpack_log;
// Host hook before MSCNT reads VU1 TPC (thread-local: set only on stage B; null by default).
extern thread_local void (*vif1_mscnt_barrier)();
}
