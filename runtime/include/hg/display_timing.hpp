#pragma once
#include <cstdint>
#include <limits>
#include <stdexcept>
namespace hg {
// Interlaced NTSC: 27 MHz timing units, 858 units per half-line,
// 525 half-lines per field, with 480 half-lines of active video.
// This is a scan timing producer, independent of drawing and host presentation.
class DisplayTiming {
    bool configured=false,blank=false;
    std::uint32_t interlace=0,mode=0,frame=0;
    std::uint64_t phase=0;
public:
    std::uint64_t fields=0;
    bool in_vblank() const {return blank;}
    void configure(bool enabled,std::uint32_t il,std::uint32_t md,std::uint32_t fr) {
        if(enabled && (il!=1 || md!=2 || fr>1))
            throw std::runtime_error("unsupported display timing mode");
        if(configured==enabled && interlace==il && mode==md && frame==fr)return;
        configured=enabled;interlace=il;mode=md;frame=fr;phase=0;blank=false;fields=0;
    }
    template<class Edge> void advance_us(std::uint64_t us,Edge edge) {
        if(!configured)return;
        if(us>std::numeric_limits<std::uint64_t>::max()/27)
            throw std::runtime_error("display clock interval overflow");
        auto ticks=us*27;
        constexpr std::uint64_t active_ticks=480*858,field_ticks=525*858;
        while(ticks) {
            const auto boundary=blank?field_ticks:active_ticks;
            const auto step=boundary-phase;
            if(ticks<step){phase+=ticks;break;}
            ticks-=step;phase=boundary;
            if(!blank){blank=true;edge(true);}
            else {blank=false;phase=0;++fields;edge(false);}
        }
    }
};
}
