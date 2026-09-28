#pragma once
#include "hg/gs.hpp"
#include <bitset>

namespace hg {
// Conservative output footprint for a clipped CPU raster rectangle. Inputs and
// read/modify/write destinations still receive the original full readback.
inline std::bitset<512> gs_raster_write_pages(const GsRegisterState& gs,unsigned context,
                                            int left,int top,int right,int bottom) {
    std::bitset<512> pages;
    if(context>1||gs.vram.size()!=1024*1024||left<0||top<0||right>2048||bottom>2048)
        return pages.set();
    if(left>=right||top>=bottom)return pages;
    const auto frame=gs.value[0x4c+context],zbuf=gs.value[0x4e+context];
    const auto width=unsigned((frame>>16)&63)*64;
    const auto fp=unsigned((frame>>24)&63),zp=unsigned((zbuf>>24)&15);
    if(!width||(fp!=0&&fp!=1&&fp!=2&&fp!=10&&fp!=0x31)||
       (zp!=0&&zp!=1&&zp!=2&&zp!=10))return pages.set();
    const auto append=[&](unsigned base,unsigned format,bool depth) {
        const unsigned height=(format==2||format==10)?64:32;
        const auto address=depth?(format==2?GsRegisterState::psmz16_word:
            format==10?GsRegisterState::psmz16s_word:GsRegisterState::psmz32_word):
            (format==2?GsRegisterState::psmct16_word:format==10?GsRegisterState::psmct16s_word:
             format==0x31?GsRegisterState::psmz32_word:GsRegisterState::psmct32_word);
        // FRAME/ZBUF bases are page aligned. The swizzles' low11 word bits
        // vary within one tile; their page selection includes physical wrap.
        for(unsigned y=unsigned(top)/height;y<=unsigned(bottom-1)/height;++y)
            for(unsigned x=unsigned(left)/64;x<=unsigned(right-1)/64;++x)
                pages.set(address(base,width,x*64,y*height)/2048);
    };
    append(unsigned(frame&511)*2048,fp,false);
    append(unsigned(zbuf&511)*2048,zp,true);
    return pages;
}
}
