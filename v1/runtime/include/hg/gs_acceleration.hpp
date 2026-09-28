#pragma once
#include <cstdint>
#include "hg/gs_memory.hpp"

namespace hg {
// Host-only acceleration; synchronous outside an explicit pending-draw batch.
// Flush before CPU raster access, and end_batch before returning to the guest.
// Resident backends may retain logically completed host work across batches only
// through GsLocalMemory observers that order/materialize all CPU consumers. Returning
// false must leave that draw's VRAM untouched (earlier queued draws may complete).
// Addresses and filtering fractions come from the checked CPU sprite setup.
struct GsSpriteAxis {std::uint32_t source0,source1,destination,fraction;};
struct GsSpriteJob {
    std::uint32_t width,height,rgba,function,flags,alpha,alpha_fixed,write_mask;
    const GsSpriteAxis* columns;
    const GsSpriteAxis* rows;
    std::uint32_t source_format=0,destination_format=0;
    std::uint64_t texa=0;
    std::uint32_t first_x=0,first_y=0,frame_width=0,zbase=0,zformat=0,z=0;
    std::uint64_t test=0x30000,dimx=0;
    bool dither=false,zwrite=false;
    const std::uint32_t* palette=nullptr;
};
struct GsSpriteAccelerator {
    virtual ~GsSpriteAccelerator()=default;
    virtual bool render(const GsSpriteJob&,GsLocalMemory& vram)=0;
    virtual void begin_batch() {}
    virtual void flush() {}
    virtual void end_batch() {}
    virtual void finish() {flush();}
};
// One AOT worker owns its context and accelerator; no guest-state additions.
inline thread_local GsSpriteAccelerator* gs_sprite_accelerator=nullptr;
}
