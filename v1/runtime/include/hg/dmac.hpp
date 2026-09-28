#pragma once
#include <array>
#include <cstdint>
#include <stdexcept>

namespace hg {
// Independent register model from EE User's Manual §§5.4, 5.8, 5.9.
// SIF0 receives explicit qwords. Other transfer endpoints remain unavailable.
struct Dmac {
    static constexpr std::array<std::uint32_t,10> bases{
        0x10008000,0x10009000,0x1000a000,0x1000b000,0x1000b400,
        0x1000c000,0x1000c400,0x1000c800,0x1000d000,0x1000d400};
    struct Channel {std::uint32_t chcr=0,madr=0,qwc=0,tadr=0,asr0=0,asr1=0,sadr=0;};
    std::array<Channel,10> channels{};
    std::uint32_t status=0,mask=0;
    // Native firmware profile starts DMA enabled. This is not hardware reset.
    std::uint32_t control=1,priority=0,interleave=0,ring_size=0,ring_offset=0,stall_address=0;
    bool held=false;
    bool enabled(unsigned channel) const {
        return !held && (control&1) && (!(priority&0x80000000u) || (priority&(1u<<(16+channel))));
    }
    bool sif0_packet=false;
    std::uint64_t sif0_completion_count=0;
    bool sif1_packet=false,sif1_end=false;
    bool gif_packet=false,gif_end=false;
    bool vif1_packet=false,vif1_end=false;
    bool ipu_input_packet=false,ipu_input_end=false;
    bool spr_input_packet=false,spr_input_end=false;
    static bool contains(std::uint32_t a) {
        return (a>=0x10008000 && a<0x1000e070) || a==0x1000f520 || a==0x1000f590;
    }
    bool pending() const {return (status&mask)!=0 || (status&0x8000)!=0;}
    void raise(std::uint32_t bits) {status|=bits&0xe3ff;}
    void finish_sif0_packet() {
        auto& c=channels[5];
        const auto tag=c.chcr>>16;
        if(((tag>>12)&7)==7 || ((tag&0x8000) && (c.chcr&0x80))) {
            c.chcr&=~0x100u;raise(1u<<5);++sif0_completion_count;
        }
        sif0_packet=false;
    }
    // One peripheral qword per call; lack of input leaves STR set and RAM intact.
    // write_quad must validate its entire destination before changing memory.
    template<class WriteQuad>
    void receive_sif0(std::uint64_t low,std::uint64_t high,WriteQuad write_quad) {
        auto& c=channels[5];
        if(!enabled(5) || !(c.chcr&0x100) || (c.chcr&0xc)!=4)
            throw std::runtime_error("SIF0 receive without active destination chain");
        if(c.qwc) {
            write_quad(c.madr,low,high);
            c.madr+=16;--c.qwc;
            if(!c.qwc && sif0_packet)finish_sif0_packet();
            return;
        }
        const auto tag=std::uint32_t(low),id=(tag>>28)&7;
        if(tag&0x0c000000)throw std::runtime_error("DMA tag priority control is not implemented");
        if(id==0)throw std::runtime_error("DMA cnts requires implemented stall control");
        c.chcr=(c.chcr&0xffff)|(tag&0xffff0000);
        if(id!=1 && id!=7) {c.chcr&=~0x100u;raise(1u<<5);return;}
        c.madr=std::uint32_t(low>>32)&0xfffffff0;
        c.qwc=tag&0xffff;sif0_packet=true;
        if(!c.qwc)finish_sif0_packet();
    }
    // Source-chain subset from the EE manual: cnt, next, ref, refe, end.
    // Tag transfer and stalls are rejected until their endpoints are implemented.
    template<class ReadQuad,class PushQuad>
    bool pump_sif1(ReadQuad read,PushQuad push) {
        auto& c=channels[6];
        if(!enabled(6) || !(c.chcr&0x100))return false;
        if(!sif1_packet) {
            const auto tag=read(c.tadr);
            const auto control=std::uint32_t(tag.low),address=std::uint32_t(tag.low>>32),id=(control>>28)&7;
            if(control&0x0c000000)throw std::runtime_error("SIF1 priority tag unsupported");
            if(id!=0 && id!=1 && id!=2 && id!=3 && id!=7)throw std::runtime_error("SIF1 source tag requires unsupported stall/stack");
            c.chcr=(c.chcr&0xffff)|(control&0xffff0000);c.qwc=control&0xffff;
            c.madr=(id==0 || id==3)?address:c.tadr+16;
            c.tadr=id==2?address:(id==1 || id==7)?c.tadr+16+c.qwc*16:c.tadr+16;
            sif1_end=id==0 || id==7 || ((control&0x80000000) && (c.chcr&0x80));
            sif1_packet=true;
        }
        if(c.qwc) {
            const auto quad=read(c.madr);
            if(!push(quad))return false;
            c.madr+=16;--c.qwc;
        }
        if(!c.qwc) {
            if(sif1_end){c.chcr&=~0x100u;raise(1u<<6);}
            sif1_packet=false;
        }
        return true;
    }
    // GIF channel 2, normal and source-chain data movement. The sink controls
    // backpressure, so a full GIF packet assembler can retain partial input.
    template<class ReadQuad,class PushQuad>
    bool pump_gif(ReadQuad read,PushQuad push) {
        auto& c=channels[2];
        if(!enabled(2) || !(c.chcr&0x100))return false;
        const auto mode=c.chcr&0xc;
        if(mode==0) {
            if(!c.qwc){c.chcr&=~0x100u;raise(1u<<2);return false;}
            const auto quad=read(c.madr);
            if(!push(quad))return false;
            c.madr+=16;
            if(--c.qwc==0){c.chcr&=~0x100u;raise(1u<<2);}
            return true;
        }
        if(mode!=4)throw std::runtime_error("GIF DMA requires unsupported interleave mode");
        if(!gif_packet) {
            const auto tag=read(c.tadr);
            const auto control=std::uint32_t(tag.low),address=std::uint32_t(tag.low>>32),id=(control>>28)&7;
            if(control&0x0c000000)throw std::runtime_error("GIF DMA priority tag unsupported");
            if(id!=0 && id!=1 && id!=2 && id!=3 && id!=7)
                throw std::runtime_error("GIF DMA source tag requires unsupported stall/stack");
            c.chcr=(c.chcr&0xffff)|(control&0xffff0000);c.qwc=control&0xffff;
            c.madr=(id==0 || id==3)?address:c.tadr+16;
            c.tadr=id==2?address:(id==1 || id==7)?c.tadr+16+c.qwc*16:c.tadr+16;
            gif_end=id==0 || id==7 || ((control&0x80000000) && (c.chcr&0x80));
            gif_packet=true;
        }
        if(c.qwc) {
            const auto quad=read(c.madr);
            if(!push(quad))return false;
            c.madr+=16;--c.qwc;
        }
        if(!c.qwc) {
            if(gif_end){c.chcr&=~0x100u;raise(1u<<2);}
            gif_packet=false;
        }
        return true;
    }
    // VIF1 channel 1 uses the documented normal/source-chain forms. VIFcode
    // processing itself is delegated to the bounded VIF1 endpoint. CALL/RET
    // use the channel's two-entry address stack; REFS still needs stall control.
    template<class ReadQuad,class PushQuad,class PushTag>
    bool pump_vif1(ReadQuad read,PushQuad push,PushTag push_tag) {
        auto& c=channels[1];
        if(!enabled(1) || !(c.chcr&0x100))return false;
        const auto mode=c.chcr&0xc;
        if(mode==0) {
            if(!c.qwc){c.chcr&=~0x100u;raise(1u<<1);return false;}
            const auto quad=read(c.madr);if(!push(quad))return false;
            c.madr+=16;if(--c.qwc==0){c.chcr&=~0x100u;raise(1u<<1);}return true;
        }
        if(mode!=4)throw std::runtime_error("VIF1 DMA requires unsupported interleave mode");
        if(!vif1_packet) {
            const auto tag_address=c.tadr;
            const auto tag=read(tag_address);const auto control=std::uint32_t(tag.low),address=std::uint32_t(tag.low>>32),id=(control>>28)&7;
            if(control&0x0c000000)throw std::runtime_error("VIF1 DMA priority tag unsupported");
            if(id==4)throw std::runtime_error("VIF1 DMA REFS tag requires stall control");
            const auto asp=(c.chcr>>4)&3u;
            if(id==5 && asp>=2)throw std::runtime_error("VIF1 DMA CALL stack overflow requires interrupt semantics");
            if((c.chcr&0x40) && !push_tag(tag.high))return false;
            c.chcr=(c.chcr&0xffff)|(control&0xffff0000);c.qwc=control&0xffff;
            const auto inline_data=tag_address+16;
            const auto continuation=inline_data+c.qwc*16;
            c.madr=(id==0 || id==3)?address:inline_data;
            if(id==2)c.tadr=address;
            else if(id==1 || id==7)c.tadr=continuation;
            else if(id==5) {
                (asp?c.asr1:c.asr0)=continuation;
                c.chcr=(c.chcr&~0x30u)|((asp+1u)<<4);
                c.tadr=address;
            } else if(id==6) {
                if(asp) {
                    const auto popped=asp==1?c.asr0:c.asr1;
                    c.chcr=(c.chcr&~0x30u)|((asp-1u)<<4);
                    c.tadr=popped;
                } else c.tadr=continuation;
            } else c.tadr=tag_address+16;
            vif1_end=id==0 || id==7 || (id==6 && !asp) || ((control&0x80000000) && (c.chcr&0x80));vif1_packet=true;
        }
        if(c.qwc){const auto quad=read(c.madr);if(!push(quad))return false;c.madr+=16;--c.qwc;}
        if(!c.qwc){if(vif1_end){c.chcr&=~0x100u;raise(1u<<1);}vif1_packet=false;}
        return true;
    }
    // toIPU source chain, EE User Manual pp45-46. The sink owns FIFO pressure.
    // A restarted transfer finishes its saved MADR/QWC before reading TADR.
    template<class ReadQuad,class PushQuad>
    bool pump_ipu_input(ReadQuad read,PushQuad push) {
        auto& c=channels[4];
        if(!enabled(4) || !(c.chcr&0x100))return false;
        const auto mode=c.chcr&0xc;
        if(mode!=0 && mode!=4)throw std::runtime_error("IPU input DMA mode unsupported");
        if(mode==4 && !ipu_input_packet) {
            const auto tag=read(c.tadr);
            const auto control=std::uint32_t(tag.low),address=std::uint32_t(tag.low>>32),id=(control>>28)&7;
            if(control&0x0c000000)throw std::runtime_error("IPU DMA priority tag unsupported");
            if(id!=0 && id!=1 && id!=2 && id!=3 && id!=7)
                throw std::runtime_error("IPU DMA source tag requires unsupported stall/stack");
            c.chcr=(c.chcr&0xffff)|(control&0xffff0000);c.qwc=control&0xffff;
            c.madr=(id==0 || id==3)?address:c.tadr+16;
            c.tadr=id==2?address:(id==1 || id==7)?c.tadr+16+c.qwc*16:c.tadr+16;
            ipu_input_end=id==0 || id==7 || ((control&0x80000000u) && (c.chcr&0x80));
            ipu_input_packet=true;
        }
        if(c.qwc) {
            const auto quad=read(c.madr);
            if(!push(quad))return false;
            c.madr+=16;--c.qwc;
        }
        if(!c.qwc) {
            if(mode==0 || ipu_input_end){c.chcr&=~0x100u;raise(1u<<4);}
            ipu_input_packet=false;
        }
        return true;
    }
    std::uint32_t& channel_register(std::uint32_t a) {
        for(unsigned n=0;n<bases.size();++n) {
            if(a<bases[n] || a>=bases[n]+0x90) continue;
            auto& c=channels[n];
            switch(a-bases[n]) {
            case 0:return c.chcr;
            case 0x10:return c.madr;
            case 0x20:return c.qwc;
            case 0x30:if(n<=2 || n==4 || n==6 || n==9)return c.tadr;break;
            case 0x40:if(n<=2)return c.asr0;break;
            case 0x50:if(n<=2)return c.asr1;break;
            case 0x80:if(n>=8)return c.sadr;break;
            }
        }
        throw std::runtime_error("unimplemented or reserved DMA register");
    }
    std::uint32_t read(std::uint32_t a) {
        if(a==0x1000f520)return held?0x00010000u:0;
        if(a==0x1000e000)return control;
        if(a==0x1000e020)return priority;
        if(a==0x1000e030)return interleave;
        if(a==0x1000e040)return ring_size;
        if(a==0x1000e050)return ring_offset;
        if(a==0x1000e060)return stall_address;
        if(a==0x1000e010)return status|(mask<<16);
        return channel_register(a);
    }
    void write(std::uint32_t a,std::uint32_t v) {
        // EE User's Manual pp62,71-72. D_ENABLEW.CPND suspends every
        // channel; D_ENABLER exposes the same bit so software can restore it.
        if(a==0x1000f590){held=(v&0x00010000u)!=0;return;}
        if(a==0x1000f520)throw std::runtime_error("DMA hold-state register is read-only");
        if(a==0x1000e000) {
            if((v&0xfc) || ((v>>8)&7)>5)throw std::runtime_error("DMA control requires unsupported MFIFO/stall/release mode");
            control=v&0x7ff;return;
        }
        if(a==0x1000e020){priority=v&0x83ff03ff;return;}
        if(a==0x1000e030){interleave=v&0x00ff00ff;return;}
        if(a==0x1000e040){ring_size=v&0x7ffffff0;return;}
        if(a==0x1000e050){ring_offset=v&0x7ffffff0;return;}
        if(a==0x1000e060){stall_address=v&0x7ffffff0;return;}
        if(a==0x1000e010) {status&=~(v&0xe3ff);mask^=(v>>16)&0x63ff;return;}
        // The original generic DMA initializer clears the same field offsets
        // on every channel. Native reserved-field policy accepts zero only;
        // no register or transfer state exists for these absent fields.
        if(!v)for(unsigned n=0;n<bases.size();++n) {
            if(a<bases[n] || a>=bases[n]+0x90)continue;
            const auto offset=a-bases[n];
            if((offset==0x80 && n<8) || ((offset==0x40 || offset==0x50) && n>2)
               || (offset==0x30 && (n==3 || n==5 || n==7 || n==8)))return;
        }
        auto& reg=channel_register(a);
        for(unsigned n=0;n<bases.size();++n) {
            if(a<bases[n] || a>=bases[n]+0x90)continue;
            const auto offset=a-bases[n];
            if((channels[n].chcr&0x100) && !held)
                throw std::runtime_error("active DMA register write requires suspension support");
            if(offset==0) {
                if((v&0x100) && n==2 && ((v&0xc)!=0 && (v&0xc)!=4 || (v&0x70)))
                    throw std::runtime_error("GIF DMA start requires unsupported mode");
                if((v&0x100) && n==1 && ((v&0xc)!=0 && (v&0xc)!=4 || (v&0x30)))
                    throw std::runtime_error("VIF1 DMA start requires unsupported mode");
                if((v&0x100) && ((n==8 && (v&0x7c)) || (n==9 && ((v&0x70) || ((v&0xc)!=0 && (v&0xc)!=4)))))
                    throw std::runtime_error("SPR DMA requires supported normal/source-chain mode");
                if((v&0x100) && n==3 && (v&0x7c))
                    throw std::runtime_error("IPU output DMA requires supported normal mode");
                if((v&0x100) && n==4 && ((v&0xc)!=0 && (v&0xc)!=4 || (v&0x70)))
                    throw std::runtime_error("IPU input DMA requires supported normal/source-chain mode");
                if((v&0x100) && n!=2 && n!=1 && n!=3 && n!=4 && n<8 && ((n!=5 && n!=6) || (v&0xc)!=4 || (v&0x70)))
                    throw std::runtime_error("DMA start requires an implemented transfer endpoint");
                if(n==5)sif0_packet=false;
                if(n==6)sif1_packet=false;
                if(n==2)gif_packet=false;
                if(n==1)vif1_packet=false;
                if(n==4) {
                    ipu_input_packet=channels[4].qwc!=0;
                    const auto id=(v>>28)&7;
                    ipu_input_end=id==0 || id==7 || ((v&0x80000000u) && (v&0x80));
                }
                if(n==9) {
                    spr_input_packet=channels[9].qwc!=0;
                    const auto id=(v>>28)&7;
                    spr_input_end=id==0 || id==7 || ((v&0x80000000u) && (v&0x80));
                }
                v&=0xffff01fd;
            } else if(offset==0x20)v&=0xffff;
            else if(offset==0x80)v&=0x3ff0;
            else {v&=0xfffffff0;if(offset==0x10 && n>=8)v&=0x7fffffff;}
            reg=v;return;
        }
    }
};
}
