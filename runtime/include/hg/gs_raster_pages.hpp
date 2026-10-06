#pragma once
#include "hg/gs.hpp"
#include <bitset>

namespace hg {
// Conservative texture-read footprint for CPU raster fallback. This may
// over-include pages, but never under-includes a supported TEX0/CLAMP address.
// Unknown/invalid state returns all pages and therefore preserves legacy sync.
inline std::bitset<512> gs_raster_texture_pages(const GsRegisterState& gs,unsigned context) {
    std::bitset<512> pages;
    if(context>1||gs.vram.size()!=1024*1024)return pages.set();
    const auto tex0=gs.value[6+context],clamp=gs.value[8+context];
    const auto base=unsigned(tex0&0x3fff)*64,stride=unsigned((tex0>>14)&63)*64;
    const auto format=unsigned((tex0>>20)&63),tw=unsigned((tex0>>26)&15),th=unsigned((tex0>>30)&15);
    if(!stride||tw>10||th>10)return pages.set();
    const auto u_mode=unsigned(clamp&3),v_mode=unsigned((clamp>>2)&3);
    const auto u_min=unsigned((clamp>>4)&1023),u_max=unsigned((clamp>>14)&1023);
    const auto v_min=unsigned((clamp>>24)&1023),v_max=unsigned((clamp>>34)&1023);
    if((u_mode==2&&u_min>u_max)||(v_mode==2&&v_min>v_max))return pages.set();
    const auto maximum=[](unsigned mode,unsigned size,unsigned low,unsigned high) {
        return mode<2?size-1:mode==2?high:(low|high);
    };
    const auto max_x=maximum(u_mode,1u<<tw,u_min,u_max),max_y=maximum(v_mode,1u<<th,v_min,v_max);
    using Address=std::uint32_t(*)(std::uint32_t,std::uint32_t,std::uint32_t,std::uint32_t);
    Address address=nullptr;unsigned page_width=64,page_height=32;
    switch(format) {
    case 0:case 1:case 0x1b:case 0x24:case 0x2c:address=GsRegisterState::psmct32_word;break;
    case 2:address=GsRegisterState::psmct16_word;page_height=64;break;
    case 10:address=GsRegisterState::psmct16s_word;page_height=64;break;
    case 0x13:address=GsRegisterState::psmt8_word;page_width=128;page_height=64;break;
    case 0x14:address=GsRegisterState::psmt4_word;page_width=128;page_height=128;break;
    case 0x31:address=GsRegisterState::psmz32_word;break;
    default:return pages.set();
    }
    try {
        for(unsigned y=0;y<=max_y;y+=page_height)
            for(unsigned x=0;x<=max_x;x+=page_width)pages.set(address(base,stride,x,y)/2048);
    } catch(const std::runtime_error&) {return pages.set();}
    return pages;
}

// Conservative output footprint for a clipped CPU raster rectangle. Frame/depth
// destinations are read/modify/write state and therefore belong in visible pages.
inline std::bitset<512> gs_raster_write_pages(const GsRegisterState& gs,unsigned context,
                                            int left,int top,int right,int bottom) {
    std::bitset<512> pages;
    if(context>1||gs.vram.size()!=1024*1024||left<0||top<0||right>2048||bottom>2048)
        return pages.set();
    if(left>=right||top>=bottom)return pages;
    const auto frame=gs.value[0x4c+context],zbuf=gs.value[0x4e + context];
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
