#pragma once

#include "hg/gif.hpp"
#include <array>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace hg {

struct GsVertex {
    std::uint32_t x=0,y=0,z=0,rgba=0,fog=0;
    std::uint64_t st=0;
    std::uint32_t uv=0;
    std::uint32_t q=0x3f800000;
};
struct GsDraw {
    std::uint8_t primitive=0;
    std::uint16_t prim_state=0;
    std::uint64_t xyoffset=0;
    std::array<GsVertex,3> vertices{};
    std::uint8_t count=0;
    std::array<std::uint64_t,0x80> environment{};
    bool environment_captured=false;
};
struct GsTextureImage {
    std::uint32_t width=0,height=0;
    std::vector<std::uint32_t> rgba;
};
struct GsDisplayImage {
    std::uint32_t width=0,height=0,dx=0,dy=0,magnify_x=1,magnify_y=1;
    std::vector<std::uint32_t> rgba;
};

// Resolve an integer texel coordinate using one CLAMP axis.  This is the
// documented point-sampling coordinate operation; filtering remains a later
// renderer step.
inline std::uint32_t gs_wrap_texture_coordinate(std::int32_t coordinate,std::uint32_t size,
                                                 std::uint32_t mode,std::uint32_t minimum,
                                                 std::uint32_t maximum) {
    if(!size)throw std::runtime_error("zero GS texture dimension");
    const auto in_texture=[&](std::int64_t value) {
        if(value<0||std::uint64_t(value)>=size)throw std::runtime_error("GS CLAMP resolved outside texture dimensions");
        return std::uint32_t(value);
    };
    switch(mode) {
    case 0: { // REPEAT
        const auto wrapped=std::int64_t(coordinate)%std::int64_t(size);
        return std::uint32_t(wrapped<0?wrapped+size:wrapped);
    }
    case 1: // CLAMP
        return coordinate<0?0:in_texture(coordinate>=std::int64_t(size)?std::int64_t(size)-1:coordinate);
    case 2: // REGION_CLAMP
        if(minimum>maximum)throw std::runtime_error("invalid GS REGION_CLAMP limits");
        return in_texture(coordinate<std::int64_t(minimum)?minimum:
                          coordinate>std::int64_t(maximum)?maximum:coordinate);
    case 3: // REGION_REPEAT
        return in_texture((std::uint32_t(coordinate)&minimum)|maximum);
    default: throw std::runtime_error("invalid GS texture wrap mode");
    }
}

// Resolve one normalized STQ coordinate to an integer texel without host
// floating-point. IEEE single-precision operands are converted to an exact
// mantissa/exponent ratio; non-finite, zero-Q, and int32 overflow are explicit.
inline std::int32_t gs_stq_texel_coordinate(std::uint32_t coordinate,std::uint32_t q,std::uint32_t size) {
    if(!size||size>1024)throw std::runtime_error("invalid GS STQ texture dimension");
    const auto decode=[](std::uint32_t bits) {
        const auto exponent=(bits>>23)&0xffu,fraction=bits&0x7fffffu;
        if(exponent==0xff)throw std::runtime_error("non-finite GS STQ coordinate");
        struct Part {bool negative,zero;std::uint32_t mantissa;int scale;};
        if(!exponent)return Part{(bits>>31)!=0,fraction==0,fraction,-149};
        return Part{(bits>>31)!=0,false,fraction|0x800000u,int(exponent)-150};
    };
    const auto s=decode(coordinate),divisor=decode(q);
    if(divisor.zero)throw std::runtime_error("GS STQ division by zero");
    if(s.zero)return 0;
    auto numerator=std::uint64_t(s.mantissa)*size,denominator=std::uint64_t(divisor.mantissa);
    const auto shift=s.scale-divisor.scale;
    if(shift>=0) {
        if(shift>=64||numerator>(~std::uint64_t(0)>>shift))throw std::runtime_error("GS STQ coordinate overflow");
        numerator<<=shift;
    } else {
        const auto right=-shift;
        if(right>=64||denominator>(~std::uint64_t(0)>>right))return s.negative!=divisor.negative?-1:0;
        denominator<<=right;
    }
    const auto quotient=numerator/denominator,remainder=numerator%denominator;
    const bool negative=s.negative!=divisor.negative;
    const auto magnitude=quotient+(negative&&remainder?1:0);
    if((!negative&&magnitude>0x7fffffffull)||(negative&&magnitude>0x80000000ull))
        throw std::runtime_error("GS STQ coordinate outside signed texel range");
    return negative?std::int32_t(-std::int64_t(magnitude)):std::int32_t(magnitude);
}

// Resolve a barycentrically weighted normalized coordinate when the vertices
// share Q. The weight denominator cancels against the subsequent Q division,
// so the complete result remains an exact integer ratio. A zero third weight
// gives the two-endpoint linear case used by sprites.
inline std::int32_t gs_stq_weighted_texel_coordinate(std::uint32_t first,std::uint32_t second,
                                                    std::uint32_t q,std::int64_t first_weight,
                                                    std::int64_t second_weight,std::uint32_t size,
                                                    std::uint32_t third=0,std::int64_t third_weight=0) {
    if(!size||size>1024||(!first_weight&&!second_weight&&!third_weight))
        throw std::runtime_error("invalid GS weighted STQ interpolation");
    struct Part {bool negative,zero;std::uint32_t mantissa;int scale;};
    const auto decode=[](std::uint32_t bits) {
        const auto exponent=(bits>>23)&0xffu,fraction=bits&0x7fffffu;
        if(exponent==0xff)throw std::runtime_error("non-finite GS STQ coordinate");
        if(!exponent)return Part{(bits>>31)!=0,fraction==0,fraction,-149};
        return Part{(bits>>31)!=0,false,fraction|0x800000u,int(exponent)-150};
    };
    const auto a=decode(first),b=decode(second),c=decode(third),divisor=decode(q);
    if(divisor.zero)throw std::runtime_error("GS STQ division by zero");
    if(a.zero&&b.zero&&c.zero)return 0;
    auto common_scale=a.zero?b.zero?c.scale:b.scale:a.scale;
    if(!b.zero&&b.scale<common_scale)common_scale=b.scale;
    if(!c.zero&&c.scale<common_scale)common_scale=c.scale;
    std::int64_t sum=0;
    const auto accumulate=[&](Part part,std::int64_t weight) {
        if(part.zero||!weight)return;
        if(weight==(std::numeric_limits<std::int64_t>::min)())
            throw std::runtime_error("GS STQ interpolation overflow");
        const auto weight_negative=weight<0;
        const auto weight_magnitude=std::uint64_t(weight_negative?-weight:weight);
        if(weight_magnitude>std::uint64_t((std::numeric_limits<std::int64_t>::max)())/part.mantissa)
            throw std::runtime_error("GS STQ interpolation overflow");
        auto magnitude=weight_magnitude*part.mantissa;
        const auto shift=unsigned(part.scale-common_scale);
        if(shift>=63||magnitude>(std::uint64_t((std::numeric_limits<std::int64_t>::max)())>>shift))
            throw std::runtime_error("GS STQ interpolation exponent spread requires validation");
        magnitude<<=shift;
        const auto term=std::int64_t(magnitude);
        if(part.negative!=weight_negative) {
            if(sum<(std::numeric_limits<std::int64_t>::min)()+term)
                throw std::runtime_error("GS STQ interpolation overflow");
            sum-=term;
        } else {
            if(sum>(std::numeric_limits<std::int64_t>::max)()-term)
                throw std::runtime_error("GS STQ interpolation overflow");
            sum+=term;
        }
    };
    accumulate(a,first_weight);accumulate(b,second_weight);accumulate(c,third_weight);
    if(!sum)return 0;
    if(sum==(std::numeric_limits<std::int64_t>::min)())
        throw std::runtime_error("GS STQ interpolation overflow");
    const bool sum_negative=sum<0;
    auto numerator=std::uint64_t(sum_negative?-sum:sum);
    if((second_weight>0&&first_weight>(std::numeric_limits<std::int64_t>::max)()-second_weight)||
       (second_weight<0&&first_weight<(std::numeric_limits<std::int64_t>::min)()-second_weight))
        throw std::runtime_error("GS STQ interpolation denominator overflow");
    const auto pair_weight=first_weight+second_weight;
    if((third_weight>0&&pair_weight>(std::numeric_limits<std::int64_t>::max)()-third_weight)||
       (third_weight<0&&pair_weight<(std::numeric_limits<std::int64_t>::min)()-third_weight))
        throw std::runtime_error("GS STQ interpolation denominator overflow");
    const auto signed_weight_sum=pair_weight+third_weight;
    if(signed_weight_sum<=0)throw std::runtime_error("invalid GS STQ interpolation weight sum");
    const auto weight_sum=std::uint64_t(signed_weight_sum);
    if(weight_sum>~std::uint64_t(0)/divisor.mantissa)
        throw std::runtime_error("GS STQ interpolation denominator overflow");
    auto denominator=weight_sum*divisor.mantissa;
    if(numerator>~std::uint64_t(0)/size)throw std::runtime_error("GS STQ coordinate overflow");
    numerator*=size;
    const auto shift=common_scale-divisor.scale;
    if(shift>=0) {
        if(shift>=64||numerator>(~std::uint64_t(0)>>shift))throw std::runtime_error("GS STQ coordinate overflow");
        numerator<<=shift;
    } else {
        const auto right=-shift;
        if(right>=64||denominator>(~std::uint64_t(0)>>right))return sum_negative!=divisor.negative?-1:0;
        denominator<<=right;
    }
    const auto quotient=numerator/denominator,remainder=numerator%denominator;
    const bool negative=sum_negative!=divisor.negative;
    const auto magnitude=quotient+(negative&&remainder?1:0);
    if((!negative&&magnitude>0x7fffffffull)||(negative&&magnitude>0x80000000ull))
        throw std::runtime_error("GS STQ coordinate outside signed texel range");
    return negative?std::int32_t(-std::int64_t(magnitude)):std::int32_t(magnitude);
}

// Perspective form for two- or three-vertex primitives. Linear/barycentric
// normalization cancels between numerator and denominator:
//     (sum(w*S)/sum(w)) / (sum(w*Q)/sum(w)) == sum(w*S)/sum(w*Q).
// Both weighted sums are retained as exact signed mantissa/exponent integers.
inline std::int32_t gs_stq_perspective_texel_coordinate(const std::array<std::uint32_t,3>& coordinates,
                                                         const std::array<std::uint32_t,3>& qs,
                                                         const std::array<std::int64_t,3>& weights,
                                                         std::uint32_t size) {
    if(!size||size>1024)throw std::runtime_error("invalid GS perspective STQ dimension");
    struct Part {bool negative,zero;std::uint32_t mantissa;int scale;};
    const auto decode=[](std::uint32_t bits) {
        const auto exponent=(bits>>23)&0xffu,fraction=bits&0x7fffffu;
        if(exponent==0xff)throw std::runtime_error("non-finite GS STQ coordinate");
        if(!exponent)return Part{(bits>>31)!=0,fraction==0,fraction,-149};
        return Part{(bits>>31)!=0,false,fraction|0x800000u,int(exponent)-150};
    };
    struct Sum {bool negative=false,zero=true;std::uint64_t magnitude=0;int scale=0;};
    const auto weighted_sum=[&](const std::array<std::uint32_t,3>& values) {
        std::array<Part,3> parts{decode(values[0]),decode(values[1]),decode(values[2])};
        bool any=false;int common_scale=0;
        for(unsigned i=0;i!=3;++i)if(weights[i]&&!parts[i].zero) {
            if(!any||parts[i].scale<common_scale)common_scale=parts[i].scale;
            any=true;
        }
        if(!any)return Sum{};
        std::int64_t total=0;
        for(unsigned i=0;i!=3;++i)if(weights[i]&&!parts[i].zero) {
            if(weights[i]==(std::numeric_limits<std::int64_t>::min)())
                throw std::runtime_error("GS perspective STQ weight overflow");
            const bool negative=parts[i].negative!=(weights[i]<0);
            const auto weight=std::uint64_t(weights[i]<0?-weights[i]:weights[i]);
            if(weight>std::uint64_t((std::numeric_limits<std::int64_t>::max)())/parts[i].mantissa)
                throw std::runtime_error("GS perspective STQ sum overflow");
            auto magnitude=weight*parts[i].mantissa;
            const auto shift=unsigned(parts[i].scale-common_scale);
            if(shift>=63||magnitude>(std::uint64_t((std::numeric_limits<std::int64_t>::max)())>>shift))
                throw std::runtime_error("GS perspective STQ exponent spread requires validation");
            const auto term=std::int64_t(magnitude<<shift);
            if(negative) {
                if(total<(std::numeric_limits<std::int64_t>::min)()+term)
                    throw std::runtime_error("GS perspective STQ sum overflow");
                total-=term;
            } else {
                if(total>(std::numeric_limits<std::int64_t>::max)()-term)
                    throw std::runtime_error("GS perspective STQ sum overflow");
                total+=term;
            }
        }
        if(!total)return Sum{};
        if(total==(std::numeric_limits<std::int64_t>::min)())
            throw std::runtime_error("GS perspective STQ sum overflow");
        return Sum{total<0,false,std::uint64_t(total<0?-total:total),common_scale};
    };
    const auto numerator_sum=weighted_sum(coordinates),denominator_sum=weighted_sum(qs);
    if(denominator_sum.zero)throw std::runtime_error("GS perspective STQ division by zero");
    if(numerator_sum.zero)return 0;
    auto numerator=numerator_sum.magnitude,denominator=denominator_sum.magnitude;
    if(numerator>~std::uint64_t(0)/size)throw std::runtime_error("GS perspective STQ coordinate overflow");
    numerator*=size;
    const auto shift=numerator_sum.scale-denominator_sum.scale;
    if(shift>=0) {
        if(shift>=64||numerator>(~std::uint64_t(0)>>shift))throw std::runtime_error("GS perspective STQ coordinate overflow");
        numerator<<=shift;
    } else {
        const auto right=-shift;
        if(right>=64||denominator>(~std::uint64_t(0)>>right))
            return numerator_sum.negative!=denominator_sum.negative?-1:0;
        denominator<<=right;
    }
    const auto quotient=numerator/denominator,remainder=numerator%denominator;
    const bool negative=numerator_sum.negative!=denominator_sum.negative;
    const auto magnitude=quotient+(negative&&remainder?1:0);
    if((!negative&&magnitude>0x7fffffffull)||(negative&&magnitude>0x80000000ull))
        throw std::runtime_error("GS perspective STQ coordinate outside signed texel range");
    return negative?std::int32_t(-std::int64_t(magnitude)):std::int32_t(magnitude);
}

// GS texture-function stage (TEX0.TFX/TCC), operating on the already expanded
// 8-bit RGBA texture color and the interpolated fragment color.  The GS uses
// a 0x80 unity multiplier, hence (a*b)>>7 rather than host normalized math.
inline std::uint32_t gs_texture_function(std::uint32_t texel,std::uint32_t fragment,
                                         std::uint32_t function,bool rgba) {
    if(function>3)throw std::runtime_error("invalid GS texture function");
    const auto byte=[](std::uint32_t value,unsigned shift) {return (value>>shift)&0xff;};
    const auto multiply=[](std::uint32_t a,std::uint32_t b) {return (a*b)>>7;};
    const auto clamp=[](std::uint32_t value) {return value>0xff?0xff:value;};
    const auto ta=byte(texel,24),fa=byte(fragment,24);
    std::uint32_t result=0;
    for(unsigned shift=0;shift!=24;shift+=8) {
        const auto product=multiply(byte(texel,shift),byte(fragment,shift));
        const auto color=function==1?byte(texel,shift):
                         function>=2?clamp(product+fa):product;
        result|=color<<shift;
    }
    const auto alpha=!rgba?fa:
        function==0?multiply(ta,fa):function==1||function==3?ta:clamp(ta+fa);
    return result|(alpha<<24);
}

struct GsBlendResult { std::int32_t r=0,g=0,b=0; std::uint8_t a=0; };

// ALPHA_1/2 calculates RGB only.  Keep its potentially out-of-range result
// intact: GS applies RGB clamping later, while writing the frame buffer.
inline GsBlendResult gs_alpha_blend(std::uint32_t source,std::uint32_t destination,
                                    std::uint64_t alpha) {
    const auto byte=[](std::uint32_t value,unsigned shift) {return std::uint32_t((value>>shift)&0xff);};
    const auto source_alpha=byte(source,24),destination_alpha=byte(destination,24);
    const auto select_color=[&](std::uint32_t selector,unsigned shift) {
        if(selector==0)return std::int32_t(byte(source,shift));
        if(selector==1)return std::int32_t(byte(destination,shift));
        if(selector==2)return std::int32_t(0);
        throw std::runtime_error("reserved GS ALPHA color selector");
    };
    const auto select_alpha=[&](std::uint32_t selector) {
        if(selector==0)return std::int32_t(source_alpha);
        if(selector==1)return std::int32_t(destination_alpha);
        if(selector==2)return std::int32_t((alpha>>32)&0xff);
        throw std::runtime_error("reserved GS ALPHA alpha selector");
    };
    const auto multiply=[](std::int32_t color,std::int32_t amount) {
        const auto value=color*amount;
        return value>=0?value/128:-((-value+127)/128); // Explicit arithmetic >>7.
    };
    const auto blend_channel=[&](unsigned shift) {
        const auto a=select_color(std::uint32_t(alpha&3),shift);
        const auto b=select_color(std::uint32_t((alpha>>2)&3),shift);
        const auto c=select_alpha(std::uint32_t((alpha>>4)&3));
        const auto d=select_color(std::uint32_t((alpha>>6)&3),shift);
        return multiply(a-b,c)+d;
    };
    return {blend_channel(0),blend_channel(8),blend_channel(16),std::uint8_t(source_alpha)};
}

// FOGCOL blends each texture-function output RGB channel by the DDA fog
// coefficient F.  The GS defines both products as an unsigned >>8; alpha is
// passed through unchanged.
inline std::uint32_t gs_apply_fog(std::uint32_t source,std::uint8_t fog,std::uint32_t fogcol) {
    const auto channel=[&](unsigned shift) {
        const auto source_channel=(source>>shift)&0xff;
        const auto fog_channel=(fogcol>>shift)&0xff;
        return ((std::uint32_t(fog)*source_channel)>>8)+
               ((std::uint32_t(0xff-fog)*fog_channel)>>8);
    };
    return channel(0)|(channel(8)<<8)|(channel(16)<<16)|(source&0xff000000u);
}

inline bool gs_compare_test(std::uint32_t value,std::uint32_t reference,std::uint32_t mode) {
    switch(mode) {
    case 0: return false; // NEVER
    case 1: return true;  // ALWAYS
    case 2: return value<reference;
    case 3: return value<=reference;
    case 4: return value==reference;
    case 5: return value>=reference;
    case 6: return value>reference;
    case 7: return value!=reference;
    default: throw std::runtime_error("invalid GS comparison mode");
    }
}

inline bool gs_alpha_test(std::uint8_t alpha,std::uint64_t test) {
    return !(test&1) || gs_compare_test(alpha,std::uint32_t((test>>4)&0xff),std::uint32_t((test>>1)&7));
}

// destination_alpha is the destination's stored alpha bit: bit 7 for
// PSMCT32 and the sole alpha bit for PSMCT16. RGB24 always passes DATE.
inline bool gs_destination_alpha_test(bool destination_alpha,std::uint32_t frame_psm,std::uint64_t test) {
    if(!(test&(1ull<<14))||frame_psm==1)return true;
    if(frame_psm!=0&&frame_psm!=2&&frame_psm!=10)throw std::runtime_error("unsupported GS destination-alpha frame format");
    return destination_alpha==((test&(1ull<<15))!=0);
}

inline bool gs_depth_test(std::uint32_t source,std::uint32_t destination,std::uint64_t test) {
    if(!(test&(1ull<<16)))throw std::runtime_error("prohibited GS depth-test disablement");
    const auto mode=std::uint32_t((test>>17)&3);
    if(mode==0)return false; // NEVER
    if(mode==1)return true;  // ALWAYS
    return mode==2?source>=destination:source>destination;
}

// Minimal GS-side transport state.  This deliberately records raw A+D writes;
// register-specific rendering and VRAM effects remain separate implementation
// work and cannot be inferred from a packet parser.
struct GsRegisterState {
    std::uint64_t privileged_imr=0x1f00; // GS reset state: all five event interrupts masked.
    std::uint64_t privileged_pmode=0,privileged_smode2=0,privileged_bgcolor=0;
    std::array<std::uint64_t,2> privileged_dispfb{},privileged_display{};
    std::uint64_t privileged_busdir=0;
    std::uint32_t privileged_csr_events=0,privileged_signal_id=0,privileged_label_id=0;
    bool crtc_configured=false;
    std::uint32_t crtc_interlace=0,crtc_mode=0,crtc_frame=0;
    std::array<std::uint64_t, 0x80> value{};
    std::array<bool, 0x80> written{};
    std::uint32_t gif_q=0x3f800000; // GIF's documented per-tag initial Q = 1.0f.
    std::vector<GsVertex> vertex_queue;
    std::vector<GsDraw> draws;
    // Cumulative counts since GS reset; draws[0] follows retired_draw_count.
    // Completed history may be retired, but pending records must stay ordered.
    std::uint64_t rasterized_draw_count=0,retired_draw_count=0;
    static constexpr std::size_t max_draws=65536;
    // GS local memory is 4 MiB / one million 32-bit words. PSMCT32's physical
    // page/block/column layout is kept here, rather than pretending transfers
    // use a linear host framebuffer.
    std::vector<std::uint32_t> vram;
    // GS User's Manual 3.4.7: one shared 1 KiB CLUT temporary buffer.
    // PSMCT32 pairs halfword n with n+256; PSMCT16/16S use one halfword.
    std::array<std::uint16_t,512> clut{};
    std::array<bool,512> clut_valid{};
    std::array<std::uint32_t,2> clut_cbp{};
    std::array<bool,2> clut_cbp_valid{};
    std::uint64_t clut_load_count=0;
    bool gif_to_vram_active=false;
    bool host_transfer_completed=false;
    unsigned completed_transfer_format=0;
    std::uint32_t transfer_x=0, transfer_y=0, transfer_width=0, transfer_remaining=0;

    void write_ad(std::uint8_t address, std::uint64_t data) {
        if (address >= value.size()) throw std::runtime_error("unsupported GS A+D register address");
        if(address==0x60&&(privileged_csr_events&1))
            throw std::runtime_error("second GS SIGNAL requires pending-event stall handling");
        if(address==0x61&&gif_to_vram_active)
            throw std::runtime_error("GS FINISH during incomplete host-to-local transfer");
        if(address==6||address==7)load_clut_from_tex0(data);
        if(address==0x16||address==0x17) {
            constexpr auto mask=(std::uint64_t{0x3f}<<20)|(~std::uint64_t{0}<<37);
            load_clut_from_tex0((value[6+(address-0x16)]&~mask)|(data&mask));
        }
        value[address] = data;
        written[address] = true;
        if(address==0x60) {
            const auto mask=std::uint32_t(data>>32),id=std::uint32_t(data);
            privileged_signal_id=(privileged_signal_id&~mask)|(id&mask);privileged_csr_events|=1;
        }
        if(address==0x61) {rasterize_pending_draws();privileged_csr_events|=2;}
        if(address==0x62) {
            const auto mask=std::uint32_t(data>>32),id=std::uint32_t(data);
            privileged_label_id=(privileged_label_id&~mask)|(id&mask);
        }
        if(address==0x16||address==0x17) { // TEX2_1/2 update their TEX0 subset.
            constexpr auto tex2_mask=(std::uint64_t{0x3f}<<20)|(~std::uint64_t{0}<<37);
            const auto tex0=std::uint8_t(6+(address-0x16));
            value[tex0]=(value[tex0]&~tex2_mask)|(data&tex2_mask);
            written[tex0]=true;
        }
        if(address==0) vertex_queue.clear(); // PRIM initializes the documented queue.
        if(address==4) kick_vertex(data,true,true);   // XYZF2
        if(address==5) kick_vertex(data,false,true);  // XYZ2
        if(address==0xc) kick_vertex(data,true,false); // XYZF3
        if(address==0xd) kick_vertex(data,false,false);// XYZ3
        if (address == 0x53) begin_transfer(data);
    }

    std::size_t pending_draw_index() const {
        if(rasterized_draw_count<retired_draw_count ||
           rasterized_draw_count-retired_draw_count>draws.size())
            throw std::runtime_error("invalid GS draw-history accounting");
        return std::size_t(rasterized_draw_count-retired_draw_count);
    }

    std::size_t retire_rasterized_draws() {
        const auto completed=pending_draw_index();
        draws.erase(draws.begin(),draws.begin()+completed);
        retired_draw_count=rasterized_draw_count;
        return completed;
    }

    void emit_draw(std::uint8_t primitive,const GsVertex& a,const GsVertex& b,const GsVertex& c,unsigned count) {
        if(draws.size()>=max_draws)retire_rasterized_draws();
        if(draws.size()>=max_draws)throw std::runtime_error("GS primitive queue exceeds bounded capacity");
        // PRIM always supplies PRIM[2:0]. PRMODECONT.AC selects whether the
        // remaining drawing attributes come from PRIM or PRMODE.
        const auto prim_controls=!written[0x1a]||(value[0x1a]&1)!=0;
        const auto attributes=prim_controls?value[0]:value[0x1b];
        const auto prim_state=std::uint16_t((value[0]&7)|(attributes&0x7f8));
        const auto context=unsigned((prim_state>>9)&1);
        GsDraw draw{};
        draw.primitive=primitive;draw.prim_state=prim_state;draw.xyoffset=value[0x18+context];
        draw.vertices={a,b,c};draw.count=std::uint8_t(count);
        draw.environment=value;draw.environment_captured=true;
        draws.push_back(std::move(draw));
    }
    void kick_vertex(std::uint64_t data,bool has_fog,bool draw) {
        const auto primitive=std::uint8_t(value[0]&7);
        if(primitive==7)throw std::runtime_error("prohibited GS primitive type");
        GsVertex vertex{};
        vertex.x=std::uint32_t(data)&0xffff;vertex.y=std::uint32_t(data>>16)&0xffff;
        vertex.z=has_fog?std::uint32_t(data>>32)&0xffffff:std::uint32_t(data>>32);
        vertex.rgba=std::uint32_t(value[1]);vertex.st=value[2];vertex.uv=std::uint32_t(value[3]);
        vertex.q=std::uint32_t(value[1]>>32);
        vertex.fog=has_fog?std::uint32_t(data>>56)&0xff:std::uint32_t(value[0xa])&0xff;
        vertex_queue.push_back(vertex);
        if(!draw)return;
        switch(primitive) {
        case 0: emit_draw(primitive,vertex,{}, {},1);vertex_queue.clear();return;
        case 1: case 6:
            if(vertex_queue.size()>=2) {emit_draw(primitive,vertex_queue[0],vertex_queue[1],{},2);vertex_queue.clear();}return;
        case 2:
            if(vertex_queue.size()>=2) {emit_draw(primitive,vertex_queue[vertex_queue.size()-2],vertex_queue.back(),{},2);vertex_queue.erase(vertex_queue.begin(),vertex_queue.end()-1);}return;
        case 3:
            if(vertex_queue.size()>=3) {emit_draw(primitive,vertex_queue[0],vertex_queue[1],vertex_queue[2],3);vertex_queue.clear();}return;
        case 4:
            if(vertex_queue.size()>=3) {emit_draw(primitive,vertex_queue[vertex_queue.size()-3],vertex_queue[vertex_queue.size()-2],vertex_queue.back(),3);vertex_queue.erase(vertex_queue.begin(),vertex_queue.end()-2);}return;
        case 5:
            if(vertex_queue.size()>=3) {emit_draw(primitive,vertex_queue[0],vertex_queue[vertex_queue.size()-2],vertex_queue.back(),3);const auto first=vertex_queue.front(),last=vertex_queue.back();vertex_queue={first,last};}return;
        default:throw std::runtime_error("prohibited GS primitive type");
        }
    }

    void write_packed(std::uint8_t descriptor, std::uint64_t low, std::uint64_t high) {
        switch (descriptor) {
        case 0: write_ad(0, low & 0x7ff); return;
        case 1: { // RGBA uses GIF's per-tag internal Q.
            const auto rgba = (low & 0xff) | ((low >> 24) & 0xff00) |
                              ((high & 0xff) << 16) | ((high >> 8) & 0xff000000);
            write_ad(1, (std::uint64_t(gif_q)<<32) | rgba); return;
        }
        case 2: // STQ
            write_ad(2, low); gif_q=std::uint32_t(high); return;
        case 3: // UV
            write_ad(3, (low & 0x3fff) | ((low >> 16) & 0x3fff0000)); return;
        case 0xa: write_ad(0xa, (high >> 36) & 0xff); return;
        case 0xe: write_ad(std::uint8_t(high), low); return;
        case 4: { // XYZF2 / XYZF3 selected by packed ADC bit 111.
            const auto data=(low&0xffff)|(((low>>32)&0xffff)<<16)|((high&0xffffff)<<32)|(((high>>32)&0xff)<<56);
            write_ad((high&(1ull<<47))?0xc:4,data);return;
        }
        case 5: { // XYZ2 / XYZ3 selected by packed ADC bit 111.
            const auto data=(low&0xffff)|(((low>>32)&0xffff)<<16)|(high<<32);
            write_ad((high&(1ull<<47))?0xd:5,data);return;
        }
        case 0xb: throw std::runtime_error("reserved GIF PACKED register descriptor");
        case 0xc: write_ad(0xc,low);return; // XYZF3, lower 64-bit direct form
        case 0xd: write_ad(0xd,low);return; // XYZ3, lower 64-bit direct form
        default: throw std::runtime_error("unsupported GIF PACKED register descriptor");
        }
    }

    // REGLIST carries one 64-bit register datum per descriptor. Unlike PACKED,
    // it has no separate address byte, so only descriptors with an unambiguous
    // GS register destination are accepted here. Vertex descriptors must not
    // accidentally turn into state-only writes: their write is also a draw kick.
    void write_reglist(std::uint8_t descriptor, std::uint64_t data) {
        switch (descriptor) {
        case 0: write_ad(0, data & 0x7ff); return; // PRIM
        case 1: write_ad(1, data); return;         // RGBAQ
        case 2: write_ad(2, data); return;         // ST
        case 3: write_ad(3, data & 0x3fff3fffull); return; // UV
        case 6: case 7: case 8: case 9:            // TEX0/CLAMP context 1/2
            write_ad(descriptor, data); return;
        case 0xa: write_ad(0xa, data & 0xff); return; // FOG
        case 4: case 5: case 0xc: case 0xd:
            write_ad(descriptor,data);return;
        case 0xe: return; // REGLIST A+D is a documented NOP.
        default: throw std::runtime_error("unsupported GIF REGLIST register descriptor");
        }
    }

    void ensure_vram() {
        if (vram.empty()) vram.resize(1024 * 1024);
        if (vram.size() != 1024 * 1024) throw std::runtime_error("invalid GS local-memory allocation");
    }

    static bool privileged_contains(std::uint32_t address) {
        return address>=0x12000000u&&address<0x14000000u;
    }
    std::uint64_t read_privileged(std::uint32_t address) const {
        if(!privileged_contains(address)||address%16)
            throw std::runtime_error("invalid GS privileged register address");
        const auto reg=(address&0x1000u)|(address&0x3f0u);
        if(reg==0x1000)return privileged_csr_events|(1ull<<14);
        if(reg==0x1080)return privileged_signal_id|(std::uint64_t(privileged_label_id)<<32);
        throw std::runtime_error("unimplemented GS privileged register read");
    }
    bool interrupt_pending() const {
        return (privileged_csr_events&(~std::uint32_t(privileged_imr>>8))&0x1f)!=0;
    }
    void reset_system() {
        // Restore the native initial-state profile. The consulted manual does not
        // enumerate every register default. Local memory contents are retained.
        GsRegisterState initial;
        initial.vram=std::move(vram);
        *this=std::move(initial);
    }
    void write_privileged(std::uint32_t address,std::uint64_t data) {
        if(!privileged_contains(address)||address%16)
            throw std::runtime_error("invalid GS privileged register address");
        // EE address bits 12 and 9:4 form the GS's seven-bit privileged
        // register address; higher regions are documented mirrors.
        const auto reg=(address&0x1000u)|(address&0x3f0u);
        switch(reg) {
        case 0x000:privileged_pmode=data;return;
        case 0x020:privileged_smode2=data;return;
        case 0x070:privileged_dispfb[0]=data;return;
        case 0x080:privileged_display[0]=data;return;
        case 0x090:privileged_dispfb[1]=data;return;
        case 0x0a0:privileged_display[1]=data;return;
        case 0x0e0:privileged_bgcolor=data;return;
        case 0x1000:
            if(data&(1ull<<8))throw std::runtime_error("unimplemented GS CSR FLUSH");
            if(data&(1ull<<9)){reset_system();return;}
            privileged_csr_events&=~(std::uint32_t(data)&0x1f);return;
        case 0x1010:privileged_imr=data;return;
        case 0x1040:
            if(data&~1ull)throw std::runtime_error("invalid GS BUSDIR value");
            privileged_busdir=data;return;
        default:throw std::runtime_error("unimplemented GS privileged register write");
        }
    }

    GsDisplayImage display_image() const {
        const auto enabled=unsigned(privileged_pmode&3);
        if(enabled!=1&&enabled!=2)
            throw std::runtime_error(enabled?"GS dual-circuit display merge is unimplemented":"GS display circuit is disabled");
        const auto circuit=enabled==1?0u:1u;
        const auto frame=privileged_dispfb[circuit],display=privileged_display[circuit];
        const auto base=std::uint32_t(frame&0x1ff)*2048;
        const auto buffer_width=std::uint32_t((frame>>9)&0x3f)*64;
        const auto format=unsigned((frame>>15)&0x1f);
        const auto source_x=std::uint32_t((frame>>32)&0x7ff),source_y=std::uint32_t((frame>>43)&0x7ff);
        const auto magnify_x=std::uint32_t((display>>23)&0xf)+1;
        const auto magnify_y=std::uint32_t((display>>27)&3)+1;
        const auto display_width=std::uint32_t((display>>32)&0xfff)+1;
        const auto display_height=std::uint32_t((display>>44)&0x7ff)+1;
        if(display_width%magnify_x||display_height%magnify_y)
            throw std::runtime_error("GS DISPLAY dimensions are not integral source pixels");
        const auto width=display_width/magnify_x,height=display_height/magnify_y;
        if(!width||!height||std::uint64_t(width)*height>1024u*1024u)
            throw std::runtime_error("GS display image exceeds bounded capacity");
        if(format!=0&&format!=1&&format!=2&&format!=10)
            throw std::runtime_error("unsupported GS display pixel format");
        if(!buffer_width)throw std::runtime_error("invalid GS display buffer width");
        GsDisplayImage result{width,height,std::uint32_t(display&0xfff),std::uint32_t((display>>12)&0x7ff),
                              magnify_x,magnify_y,std::vector<std::uint32_t>(std::size_t(width)*height)};
        const auto expand=[](std::uint32_t value){return (value<<3)|(value>>2);};
        for(std::uint32_t y=0;y<height;++y)for(std::uint32_t x=0;x<width;++x) {
            std::uint32_t pixel;
            if(format==0)pixel=pixel32(base,buffer_width,source_x+x,source_y+y);
            else if(format==1)pixel=(pixel32(base,buffer_width,source_x+x,source_y+y)&0xffffffu)|0x80000000u;
            else {
                const auto packed=format==2?pixel16(base,buffer_width,source_x+x,source_y+y):
                                             pixel16s(base,buffer_width,source_x+x,source_y+y);
                pixel=expand(packed&31)|(expand((packed>>5)&31)<<8)|(expand((packed>>10)&31)<<16)|
                      ((packed&0x8000)?0x80000000u:0u);
            }
            result.rgba[std::size_t(y)*width+x]=pixel;
        }
        return result;
    }

    static std::uint32_t psmct32_word(std::uint32_t base_words, std::uint32_t buffer_width,
                                      std::uint32_t x, std::uint32_t y) {
        if (!buffer_width || buffer_width % 64) throw std::runtime_error("invalid PSMCT32 buffer width");
        x &= 2047; y &= 2047;
        const std::uint32_t page = (x / 64) + (y / 32) * (buffer_width / 64);
        const std::uint32_t block_x = (x / 8) & 7, block_y = (y / 8) & 3;
        const std::uint32_t block = (block_x & 1) + ((block_x & 2) * 2) + ((block_x & 4) * 4) +
                                    ((block_y & 1) * 2) + ((block_y & 2) * 4);
        const std::uint32_t column = (y & 7) / 2;
        const std::uint32_t pixel = (x & 1) + ((x & 6) * 2) + ((y & 1) * 2);
        return (base_words + page * 2048 + block * 64 + column * 16 + pixel) & 0xfffffu;
    }

    static std::uint32_t psmct16_word(std::uint32_t base_words, std::uint32_t buffer_width,
                                      std::uint32_t x, std::uint32_t y) {
        if (!buffer_width || buffer_width % 64) throw std::runtime_error("invalid PSMCT16 buffer width");
        x &= 2047; y &= 2047;
        const std::uint32_t page = (x / 64) + (y / 64) * (buffer_width / 64);
        const std::uint32_t block_x = (x / 16) & 3, block_y = (y / 8) & 7;
        const std::uint32_t block = (block_x & 1) * 2 + (block_x & 2) * 4 +
                                    (block_y & 1) + (block_y & 2) * 2 + (block_y & 4) * 4;
        const std::uint32_t column = (y & 7) / 2;
        const std::uint32_t pixel = (x & 1) + ((x & 6) * 2) + ((y & 1) * 2);
        return (base_words + page * 2048 + block * 64 + column * 16 + pixel) & 0xfffffu;
    }

    static std::uint32_t psmct16s_word(std::uint32_t base_words,std::uint32_t buffer_width,
                                       std::uint32_t x,std::uint32_t y) {
        if(!buffer_width||buffer_width%64)throw std::runtime_error("invalid PSMCT16S buffer width");
        x&=2047;y&=2047;
        const std::uint32_t page=(x/64)+(y/64)*(buffer_width/64);
        static constexpr std::uint8_t blocks[32]={
            0,2,16,18,1,3,17,19,8,10,24,26,9,11,25,27,
            4,6,20,22,5,7,21,23,12,14,28,30,13,15,29,31};
        const auto block=blocks[((y/8)&7)*4+((x/16)&3)];
        const auto column=(y&7)/2;
        const auto pixel=(x&1)+((x&6)*2)+((y&1)*2);
        return (base_words+page*2048+block*64+column*16+pixel)&0xfffffu;
    }

    static std::uint32_t psmz32_word(std::uint32_t base_words,std::uint32_t buffer_width,
                                     std::uint32_t x,std::uint32_t y) {
        if(!buffer_width||buffer_width%64)throw std::runtime_error("invalid PSMZ32 buffer width");
        x&=2047;y&=2047;
        const std::uint32_t page=(x/64)+(y/32)*(buffer_width/64);
        // GS Manual 8.3.1: Z32 uses a different block order from CT32 while
        // retaining its 8x8 block and 8x2 column pixel ordering.
        static constexpr std::uint8_t blocks[32]={
            24,25,28,29,8,9,12,13,26,27,30,31,10,11,14,15,
            16,17,20,21,0,1,4,5,18,19,22,23,2,3,6,7};
        const auto block=blocks[((y/8)&3)*8+((x/8)&7)];
        const auto column=(y&7)/2;
        const auto pixel=(x&1)+((x&6)*2)+((y&1)*2);
        return (base_words+page*2048+block*64+column*16+pixel)&0xfffffu;
    }

    static std::uint32_t psmz16_word(std::uint32_t base_words,std::uint32_t buffer_width,
                                     std::uint32_t x,std::uint32_t y) {
        if(!buffer_width||buffer_width%64)throw std::runtime_error("invalid PSMZ16 buffer width");
        x&=2047;y&=2047;
        const std::uint32_t page=(x/64)+(y/64)*(buffer_width/64);
        // GS Manual 8.3.2: Z16 changes the CT16 8x4 block order, while the
        // 16x2 column and its two 16-bit lanes retain their normal ordering.
        static constexpr std::uint8_t blocks[32]={
            24,26,16,18,25,27,17,19,28,30,20,22,29,31,21,23,
            8,10,0,2,9,11,1,3,12,14,4,6,13,15,5,7};
        const auto block=blocks[((y/8)&7)*4+((x/16)&3)];
        const auto column=(y&7)/2;
        const auto pixel=(x&1)+((x&6)*2)+((y&1)*2);
        return (base_words+page*2048+block*64+column*16+pixel)&0xfffffu;
    }

    static std::uint32_t psmz16s_word(std::uint32_t base_words,std::uint32_t buffer_width,
                                      std::uint32_t x,std::uint32_t y) {
        if(!buffer_width||buffer_width%64)throw std::runtime_error("invalid PSMZ16S buffer width");
        x&=2047;y&=2047;
        const std::uint32_t page=(x/64)+(y/64)*(buffer_width/64);
        static constexpr std::uint8_t blocks[32]={
            24,26,8,10,25,27,9,11,16,18,0,2,17,19,1,3,
            28,30,12,14,29,31,13,15,20,22,4,6,21,23,5,7};
        const auto block=blocks[((y/8)&7)*4+((x/16)&3)];
        const auto column=(y&7)/2;
        const auto pixel=(x&1)+((x&6)*2)+((y&1)*2);
        return (base_words+page*2048+block*64+column*16+pixel)&0xfffffu;
    }

    static std::uint32_t psmt8_word(std::uint32_t base_words, std::uint32_t buffer_width,
                                    std::uint32_t x, std::uint32_t y) {
        if (!buffer_width || buffer_width % 64) throw std::runtime_error("invalid PSMT8 buffer width");
        x &= 2047; y &= 2047;
        // BITBLTBUF uses 64-pixel units even for 128-pixel indexed pages.
        // Keep the whole-page row stride: the independent DBW=11 upload
        // probe observes five pages, not six or alternating half-pages.
        // See docs/ORACLE.md; TEX0's stricter alignment is checked separately.
        const std::uint32_t page = (x / 128) + (y / 64) * (buffer_width / 128);
        const std::uint32_t block_x = (x / 16) & 7, block_y = (y / 16) & 3;
        const std::uint32_t block = (block_x & 1) + ((block_x & 2) * 2) + ((block_x & 4) * 4) +
                                    ((block_y & 1) * 2) + ((block_y & 2) * 4);
        const std::uint32_t column = (y & 15) / 4;
        const std::uint32_t row = (y & 3) ^ ((column & 1) ? 2 : 0);
        const std::uint32_t pixel = ((x & 1) + ((x & 6) * 2) + ((row & 1) * 2)) ^ ((row & 2) * 4);
        return (base_words + page * 2048 + block * 64 + column * 16 + pixel) & 0xfffffu;
    }

    static std::uint32_t psmt4_word(std::uint32_t base_words, std::uint32_t buffer_width,
                                    std::uint32_t x, std::uint32_t y) {
        if (!buffer_width || buffer_width % 64) throw std::runtime_error("invalid PSMT4 buffer width");
        x &= 2047; y &= 2047;
        const std::uint32_t page = (x / 128) + (y / 128) * (buffer_width / 128);
        const std::uint32_t block_x = (x / 32) & 3, block_y = (y / 16) & 7;
        const std::uint32_t block = (block_x & 1) * 2 + (block_x & 2) * 4 +
                                    (block_y & 1) + (block_y & 2) * 2 + (block_y & 4) * 4;
        const std::uint32_t column = (y & 15) / 4;
        const std::uint32_t row = (y & 3) ^ ((column & 1) ? 2 : 0);
        const std::uint32_t pixel = ((x & 1) + ((x & 6) * 2) + ((row & 1) * 2)) ^ ((row & 2) * 4);
        return (base_words + page * 2048 + block * 64 + column * 16 + pixel) & 0xfffffu;
    }

    std::uint32_t pixel32(std::uint32_t base_words, std::uint32_t buffer_width,
                          std::uint32_t x, std::uint32_t y) const {
        if (vram.size() != 1024 * 1024) throw std::runtime_error("GS local memory is not initialized");
        return vram[psmct32_word(base_words, buffer_width, x, y)];
    }
    std::uint16_t pixel16(std::uint32_t base_words, std::uint32_t buffer_width,
                          std::uint32_t x, std::uint32_t y) const {
        if (vram.size() != 1024 * 1024) throw std::runtime_error("GS local memory is not initialized");
        const auto word = vram[psmct16_word(base_words, buffer_width, x, y)];
        return std::uint16_t(word >> ((x & 8) ? 16 : 0));
    }
    std::uint16_t pixel16s(std::uint32_t base_words,std::uint32_t buffer_width,
                           std::uint32_t x,std::uint32_t y) const {
        if(vram.size()!=1024*1024)throw std::runtime_error("GS local memory is not initialized");
        const auto word=vram[psmct16s_word(base_words,buffer_width,x,y)];
        return std::uint16_t(word>>((x&8)?16:0));
    }
    std::uint8_t pixel8(std::uint32_t base_words, std::uint32_t buffer_width,
                        std::uint32_t x, std::uint32_t y) const {
        if (vram.size() != 1024 * 1024) throw std::runtime_error("GS local memory is not initialized");
        const auto word = vram[psmt8_word(base_words, buffer_width, x, y)];
        const auto lane = ((y & 2) >> 1) | ((x & 8) >> 2);
        return std::uint8_t(word >> (lane * 8));
    }
    std::uint8_t pixel4(std::uint32_t base_words, std::uint32_t buffer_width,
                        std::uint32_t x, std::uint32_t y) const {
        if (vram.size() != 1024 * 1024) throw std::runtime_error("GS local memory is not initialized");
        const auto word = vram[psmt4_word(base_words, buffer_width, x, y)];
        const auto lane = ((y & 2) >> 1) | ((x & 24) >> 2);
        return std::uint8_t(word >> (lane * 4)) & 0xf;
    }

    static unsigned transfer_pixel_depth(unsigned format) {
        switch(format) {
        case 0: case 0x30:return 32;
        case 1: case 0x31:return 24;
        case 2: case 10: case 0x32: case 0x3a:return 16;
        case 0x13: case 0x1b:return 8;
        case 0x14: case 0x24: case 0x2c:return 4;
        default:throw std::runtime_error("unsupported GS transfer pixel format");
        }
    }
    static void validate_transfer_buffer_width(unsigned format,std::uint32_t width) {
        (void)transfer_pixel_depth(format);
        if(!width||width>2048||width%64)
            throw std::runtime_error("invalid GS transfer buffer width");
    }
    static unsigned transfer_x_alignment(unsigned format) {
        return format==0x13||format==0x1b?2u:
               format==0x14||format==0x24||format==0x2c?4u:1u;
    }
    std::uint32_t read_transfer_pixel(unsigned format,std::uint32_t base,std::uint32_t width,
                                      std::uint32_t x,std::uint32_t y) const {
        if(vram.size()!=1024*1024)throw std::runtime_error("GS local memory is not initialized");
        if(format==0||format==1)return vram[psmct32_word(base,width,x,y)]&(format==1?0xffffffu:0xffffffffu);
        if(format==0x30||format==0x31)return vram[psmz32_word(base,width,x,y)]&(format==0x31?0xffffffu:0xffffffffu);
        if(format==2||format==10||format==0x32||format==0x3a) {
            const auto word=(format==2?psmct16_word:format==10?psmct16s_word:
                             format==0x32?psmz16_word:psmz16s_word)(base,width,x,y);
            return std::uint16_t(vram[word]>>((x&8)?16:0));
        }
        if(format==0x13)return pixel8(base,width,x,y);
        if(format==0x14)return pixel4(base,width,x,y);
        const auto word=psmct32_word(base,width,x,y);
        if(format==0x1b)return vram[word]>>24;
        if(format==0x24)return (vram[word]>>24)&15;
        if(format==0x2c)return vram[word]>>28;
        throw std::runtime_error("unsupported GS transfer pixel format");
    }
    void write_transfer_pixel(unsigned format,std::uint32_t base,std::uint32_t width,
                              std::uint32_t x,std::uint32_t y,std::uint32_t pixel) {
        if(format==0||format==1||format==0x30||format==0x31) {
            const auto word=(format<0x30?psmct32_word:psmz32_word)(base,width,x,y);
            const auto mask=(format==1||format==0x31)?0xff000000u:0u;
            vram[word]=(vram[word]&mask)|(pixel&~mask);return;
        }
        if(format==2||format==10||format==0x32||format==0x3a) {
            const auto word=(format==2?psmct16_word:format==10?psmct16s_word:
                             format==0x32?psmz16_word:psmz16s_word)(base,width,x,y);
            const auto shift=(x&8)?16:0;
            vram[word]=(vram[word]&~(0xffffu<<shift))|((pixel&0xffffu)<<shift);return;
        }
        if(format==0x13) {
            auto& word=vram[psmt8_word(base,width,x,y)];const auto lane=((y&2)>>1)|((x&8)>>2);
            word=(word&~(0xffu<<(lane*8)))|((pixel&0xffu)<<(lane*8));return;
        }
        if(format==0x14) {
            auto& word=vram[psmt4_word(base,width,x,y)];const auto lane=((y&2)>>1)|((x&24)>>2);
            word=(word&~(0xfu<<(lane*4)))|((pixel&15u)<<(lane*4));return;
        }
        auto& word=vram[psmct32_word(base,width,x,y)];
        if(format==0x1b)word=(word&0xffffffu)|((pixel&0xffu)<<24);
        else if(format==0x24)word=(word&0xf0ffffffu)|((pixel&15u)<<24);
        else if(format==0x2c)word=(word&0x0fffffffu)|((pixel&15u)<<28);
        else throw std::runtime_error("unsupported GS transfer pixel format");
    }

    // TEX0/TEX2 accesses load a palette from VRAM into the shared temporary
    // buffer. CSA selects the destination, never the source VRAM coordinates.
    // See GS User's Manual 2.7, 3.4.7 and TEX0/TEX2 register descriptions.
    void load_clut_from_tex0(std::uint64_t tex0) {
        const auto psm=unsigned((tex0>>20)&0x3f);
        const bool four=psm==0x14||psm==0x24||psm==0x2c;
        if(!four&&psm!=0x13&&psm!=0x1b)return;
        const auto cld=unsigned(tex0>>61);
        if(!cld)return;
        if(cld>5)throw std::runtime_error("reserved GS CLUT load control");
        const auto cpsm=unsigned((tex0>>51)&15),csm=unsigned((tex0>>55)&1);
        const auto csa=unsigned((tex0>>56)&31),cbp=std::uint32_t((tex0>>37)&0x3fff);
        if(cpsm!=0&&cpsm!=2&&cpsm!=10)throw std::runtime_error("unsupported GS CLUT pixel format");
        if(csm&&(cpsm==0||csa))throw std::runtime_error("CSM2 requires a 16-bit CLUT and CSA=0");
        const auto count=four?16u:256u,offset=csa*16,capacity=cpsm==0?256u:512u;
        if(offset+count>capacity)throw std::runtime_error("GS CLUT load exceeds temporary-buffer capacity");
        if(cld>=4) {
            if(csm)throw std::runtime_error("conditional GS CLUT load requires CSM1");
            const auto slot=cld-4;
            if(!clut_cbp_valid[slot])throw std::runtime_error("GS CLUT compare register has not been initialized");
            if(clut_cbp[slot]==cbp)return;
        }
        std::uint32_t width=64,start_x=0,start_y=0;
        if(csm) {
            const auto texclut=value[0x1c];
            width=std::uint32_t(texclut&63)*64;
            start_x=std::uint32_t((texclut>>6)&63)*16;
            start_y=std::uint32_t((texclut>>12)&1023);
            if(!width||start_x+count>width)throw std::runtime_error("CSM2 CLUT load exceeds source row");
        }
        if(gif_to_vram_active)throw std::runtime_error("GS CLUT load during incomplete host-to-local transfer");
        // Older draws must see their old palette, and may themselves write the
        // new palette's VRAM source. Flush before both source reads and mutation.
        rasterize_pending_draws();
        ensure_vram();
        std::array<std::uint32_t,256> source{};
        for(unsigned index=0;index<count;++index) {
            const auto x=csm?start_x+index:four?(index&7):((index&7)|((index&0x10)>>1));
            const auto y=csm?start_y:four?(index>>3):(((index&8)>>3)|((index&0xe0)>>4));
            source[index]=read_transfer_pixel(cpsm,cbp*64,width,x,y);
        }
        for(unsigned index=0;index<count;++index) {
            const auto entry=offset+index;
            clut[entry]=std::uint16_t(source[index]);clut_valid[entry]=true;
            if(cpsm==0) {clut[entry+256]=std::uint16_t(source[index]>>16);clut_valid[entry+256]=true;}
        }
        if(cld>=2) {
            const auto slot=(cld==2||cld==4)?0u:1u;
            clut_cbp[slot]=cbp;clut_cbp_valid[slot]=true;
        }
        ++clut_load_count;
    }

    // Direct-color texels read VRAM; indexed texels read the palette retained
    // by the most recent applicable TEX0/TEX2 load, with live TEXA expansion.
    std::uint32_t texel_from_tex0(unsigned context,std::uint32_t x,std::uint32_t y) const {
        if(context>1)throw std::runtime_error("invalid GS texture context");
        const auto tex0=value[6+context];
        const auto base=std::uint32_t(tex0&0x3fff)*64;
        const auto width_words=std::uint32_t((tex0>>14)&0x3f)*64;
        const auto psm=std::uint32_t((tex0>>20)&0x3f);
        const auto tw=unsigned((tex0>>26)&0xf),th=unsigned((tex0>>30)&0xf);
        if(tw>10||th>10||!width_words)throw std::runtime_error("invalid TEX0 dimensions");
        // Indexed addressing retains whole 128-pixel pages per row, including
        // odd TBW units. Independent synthetic sampling observations and the
        // physical-marker regressions cover the truncation/alias cases.
        const std::uint32_t width=1u<<tw,height=1u<<th;
        if(std::uint64_t(width)*height>1024u*1024u)throw std::runtime_error("TEX0 texture exceeds bounded extraction capacity");
        if(x>=width||y>=height)throw std::runtime_error("TEX0 texel outside dimensions");
        // TEXA controls the expanded alpha of RGB24/RGBA16 texels (including
        // RGBA16 values reached through a CLUT).  See GS User's Manual 3.4.6.
        const auto texa=value[0x3b];
        const auto ta0=std::uint32_t(texa&0xff),ta1=std::uint32_t((texa>>32)&0xff);
        const auto aem=(texa&(1ull<<15))!=0;
        const auto rgba16=[=](std::uint16_t value) {
            const auto expand=[](std::uint16_t channel) {return std::uint32_t((channel<<3)|(channel>>2));};
            const auto rgb=value&0x7fff;
            return expand(value&31)|(expand((value>>5)&31)<<8)|(expand((value>>10)&31)<<16)|
                   ((aem&&!rgb)?0:((value&0x8000)?ta1:ta0)<<24);
        };
        const auto rgba24=[=](std::uint32_t value) {
            const auto rgb=value&0xffffff;
            return rgb|((aem&&!rgb)?0:ta0<<24);
        };
        const auto indexed_clut=[&](std::uint8_t index) {
            const auto cpsm=std::uint32_t((tex0>>51)&0xf),csm=std::uint32_t((tex0>>55)&1),csa=std::uint32_t((tex0>>56)&0x1f);
            if(cpsm!=0&&cpsm!=2&&cpsm!=10)throw std::runtime_error("unsupported GS CLUT pixel format");
            if(csm&&(cpsm==0||csa))throw std::runtime_error("CSM2 requires a 16-bit CLUT and CSA=0");
            const auto entry=csa*16+index,capacity=cpsm==0?256u:512u;
            if(entry>=capacity)throw std::runtime_error("GS CLUT index exceeds temporary-buffer capacity");
            if(!clut_valid[entry]||(cpsm==0&&!clut_valid[entry+256]))
                throw std::runtime_error("GS indexed texture uses an unloaded CLUT entry");
            if(cpsm==0)return std::uint32_t(clut[entry])|(std::uint32_t(clut[entry+256])<<16);
            return rgba16(clut[entry]);
        };
        if(psm==0)return pixel32(base,width_words,x,y);
        else if(psm==1)return rgba24(pixel32(base,width_words,x,y));
        else if(psm==2)return rgba16(pixel16(base,width_words,x,y));
        else if(psm==10)return rgba16(pixel16s(base,width_words,x,y));
        else if(psm==0x13)return indexed_clut(pixel8(base,width_words,x,y));
        else if(psm==0x14)return indexed_clut(pixel4(base,width_words,x,y));
        else if(psm==0x1b)return indexed_clut(pixel32(base,width_words,x,y)>>24);
        else if(psm==0x24)return indexed_clut((pixel32(base,width_words,x,y)>>24)&15);
        else if(psm==0x2c)return indexed_clut(pixel32(base,width_words,x,y)>>28);
        else throw std::runtime_error("TEX0 requires unimplemented texture format");
    }
    GsTextureImage texture_from_tex0(unsigned context) const {
        if(context>1)throw std::runtime_error("invalid GS texture context");
        const auto tex0=value[6+context];
        const auto tw=unsigned((tex0>>26)&0xf),th=unsigned((tex0>>30)&0xf);
        if(tw>10||th>10)throw std::runtime_error("invalid TEX0 dimensions");
        const std::uint32_t width=1u<<tw,height=1u<<th;
        GsTextureImage image{width,height,std::vector<std::uint32_t>(std::size_t(width)*height)};
        for(std::uint32_t y=0;y<height;++y)for(std::uint32_t x=0;x<width;++x)
            image.rgba[std::size_t(y)*width+x]=texel_from_tex0(context,x,y);
        return image;
    }

    // Point-sample TEX0 after applying the context's CLAMP_1/2 fields.  The
    // single-texel decoder shares format/CLUT/TEXA checks with extraction.
    // Read live VRAM on every sample, including render-to-texture feedback.
    std::uint32_t point_sample_tex0(unsigned context,std::int32_t u,std::int32_t v) const {
        if(context>1)throw std::runtime_error("invalid GS texture context");
        const auto tex0=value[6+context];
        const auto tw=unsigned((tex0>>26)&0xf),th=unsigned((tex0>>30)&0xf);
        if(tw>10||th>10)throw std::runtime_error("invalid TEX0 dimensions");
        const auto width=1u<<tw,height=1u<<th;
        const auto clamp=value[8+context];
        const auto x=gs_wrap_texture_coordinate(u,width,std::uint32_t(clamp&3),
                                                std::uint32_t((clamp>>4)&0x3ff),std::uint32_t((clamp>>14)&0x3ff));
        const auto y=gs_wrap_texture_coordinate(v,height,std::uint32_t((clamp>>2)&3),
                                                std::uint32_t((clamp>>24)&0x3ff),std::uint32_t((clamp>>34)&0x3ff));
        return texel_from_tex0(context,x,y);
    }

    // Point-sample and execute TEX0's texture function against an interpolated
    // fragment color. This is deliberately a GS-side operation; host backends
    // receive its result only after their remaining pixel-pipeline work exists.
    std::uint32_t shade_point_tex0(unsigned context,std::int32_t u,std::int32_t v,
                                   std::uint32_t fragment) const {
        if(context>1)throw std::runtime_error("invalid GS texture context");
        const auto tex0=value[6+context];
        return gs_texture_function(point_sample_tex0(context,u,v),fragment,
                                   std::uint32_t((tex0>>35)&3),(tex0&(1ull<<34))!=0);
    }

    // Commit an already-tested/blended pixel through PSMCT32/24 or the
    // PSMCT16 path. RGB24 preserves its upper byte.
    void write_frame_pixel(unsigned context,std::uint32_t x,std::uint32_t y,const GsBlendResult& color,
                           std::uint32_t additional_mask=0) {
        if(context>1)throw std::runtime_error("invalid GS frame context");
        const auto frame=value[0x4c+context];
        const auto psm=std::uint32_t((frame>>24)&0x3f);
        if(psm!=0&&psm!=1&&psm!=2&&psm!=10)throw std::runtime_error("unimplemented GS frame-buffer pixel format");
        const auto base=std::uint32_t(frame&0x1ff)*2048;
        const auto width=std::uint32_t((frame>>16)&0x3f)*64;
        if(!width)throw std::runtime_error("invalid GS frame-buffer width");
        const auto convert=[&](std::int32_t channel) {
            if(value[0x46]&1)return std::uint32_t(channel<0?0:channel>255?255:channel);
            return std::uint32_t(channel)&0xff;
        };
        const auto rgb=convert(color.r)|(convert(color.g)<<8)|(convert(color.b)<<16);
        if(psm==2||psm==10) {
            const auto dither=[&] {
                if(!(value[0x45]&1))return 0;
                const auto raw=std::uint32_t((value[0x44]>>(((y&3)*4+(x&3))*4))&7);
                return raw&4?std::int32_t(raw)-8:std::int32_t(raw);
            }();
            const auto convert_dithered=[&](std::int32_t channel) {
                channel+=dither;
                if(value[0x46]&1)return std::uint32_t(channel<0?0:channel>255?255:channel);
                return std::uint32_t(channel)&0xff;
            };
            const auto rgb5=(convert_dithered(color.r)>>3&31)|((convert_dithered(color.g)>>3&31)<<5)|
                            ((convert_dithered(color.b)>>3&31)<<10);
            const auto packed=rgb5|
                              (((color.a&0x80)||(value[0x4a+context]&1))?0x8000u:0u);
            ensure_vram();
            auto& word=vram[(psm==2?psmct16_word:psmct16s_word)(base,width,x,y)];
            const auto shift=(x&8)?16:0;
            // FRAME.FBMSK positions correspond to the pre-conversion RGBA8
            // value: R[7:3], G[15:11], B[23:19], and A[31] map to RGB5A1.
            const auto source_mask=std::uint32_t(frame>>32)|additional_mask;
            const auto mask=std::uint16_t(((source_mask>>3)&0x1f)|((source_mask>>6)&0x3e0)|
                                          ((source_mask>>9)&0x7c00)|((source_mask>>16)&0x8000));
            const auto old=std::uint16_t(word>>shift);
            const auto written=std::uint16_t((old&mask)|(packed&~mask));
            word=(word&~(0xffffu<<shift))|(std::uint32_t(written)<<shift);
            return;
        }
        const auto alpha=std::uint32_t(color.a)|((value[0x4a+context]&1)?0x80u:0u);
        const auto pixel=rgb|(alpha<<24);
        ensure_vram();
        const auto word=psmct32_word(base,width,x,y);
        if(psm==1) {
            const auto mask=(std::uint32_t(frame>>32)|additional_mask)&0xffffffu;
            vram[word]=(vram[word]&0xff000000u)|(vram[word]&mask)|(rgb&~mask);
        } else {
            const auto mask=std::uint32_t(frame>>32)|additional_mask;
            vram[word]=(vram[word]&mask)|(pixel&~mask);
        }
    }

    std::uint32_t read_z_pixel(unsigned context,std::uint32_t x,std::uint32_t y) const {
        if(context>1)throw std::runtime_error("invalid GS Z-buffer context");
        const auto frame=value[0x4c+context],zbuf=value[0x4e + context];
        const auto width=std::uint32_t((frame>>16)&0x3f)*64;
        if(!width)throw std::runtime_error("invalid GS Z-buffer width");
        const auto psm=std::uint32_t((zbuf>>24)&0xf);
        if(psm!=0&&psm!=1&&psm!=2&&psm!=10)throw std::runtime_error("unimplemented GS Z-buffer pixel format");
        if(vram.size()!=1024*1024)throw std::runtime_error("GS local memory is not initialized");
        if(psm==2||psm==10) {
            const auto word=vram[(psm==2?psmz16_word:psmz16s_word)(std::uint32_t(zbuf&0x1ff)*2048,width,x,y)];
            return std::uint16_t(word>>((x&8)?16:0));
        }
        const auto stored=vram[psmz32_word(std::uint32_t(zbuf&0x1ff)*2048,width,x,y)];
        return psm==1?stored&0xffffffu:stored;
    }

    void write_z_pixel(unsigned context,std::uint32_t x,std::uint32_t y,std::uint32_t z) {
        if(context>1)throw std::runtime_error("invalid GS Z-buffer context");
        const auto frame=value[0x4c+context],zbuf=value[0x4e + context];
        const auto width=std::uint32_t((frame>>16)&0x3f)*64;
        if(!width)throw std::runtime_error("invalid GS Z-buffer width");
        const auto psm=std::uint32_t((zbuf>>24)&0xf);
        if(psm!=0&&psm!=1&&psm!=2&&psm!=10)throw std::runtime_error("unimplemented GS Z-buffer pixel format");
        if(zbuf&(1ull<<32))return;
        ensure_vram();
        if(psm==2||psm==10) {
            auto& word=vram[(psm==2?psmz16_word:psmz16s_word)(std::uint32_t(zbuf&0x1ff)*2048,width,x,y)];
            const auto shift=(x&8)?16:0;
            word=(word&~(0xffffu<<shift))|(std::uint32_t(std::uint16_t(z))<<shift);
            return;
        }
        auto& stored=vram[psmz32_word(std::uint32_t(zbuf&0x1ff)*2048,width,x,y)];
        if(psm==1)stored=(stored&0xff000000u)|(z&0xffffffu);else stored=z;
    }

    // Frame-buffer portion of the GS pixel path. It applies alpha failure
    // policy, DATE, optional PRIM.ABE blending, then the PSMCT32 writer.
    // Z-buffer effects intentionally remain separate until a checked Z writer
    // exists; an AFAIL=ZB_ONLY pixel therefore has no framebuffer effect here.
    bool draw_frame_pixel(unsigned context,std::uint32_t x,std::uint32_t y,std::uint32_t source,
                          std::uint16_t primitive_state=0xffff) {
        if(context>1)throw std::runtime_error("invalid GS frame context");
        const auto frame=value[0x4c+context];
        const auto psm=std::uint32_t((frame>>24)&0x3f);
        if(psm!=0&&psm!=1&&psm!=2&&psm!=10)throw std::runtime_error("unimplemented GS frame-buffer pixel format");
        const auto base=std::uint32_t(frame&0x1ff)*2048,width=std::uint32_t((frame>>16)&0x3f)*64;
        if(!width)throw std::runtime_error("invalid GS frame-buffer width");
        ensure_vram();
        const auto expand=[](std::uint32_t channel) {return (channel<<3)|(channel>>2);};
        const auto destination=[&] {
            if(psm==2||psm==10) {
                const auto packed=psm==2?pixel16(base,width,x,y):pixel16s(base,width,x,y);
                return expand(packed&31)|(expand((packed>>5)&31)<<8)|(expand((packed>>10)&31)<<16)|
                       ((packed&0x8000)?0x80000000u:0u);
            }
            const auto word=psmct32_word(base,width,x,y);
            return psm==1?(vram[word]&0xffffffu)|0x80000000u:vram[word];
        }();
        const auto test=value[0x47+context];
        const auto alpha_pass=gs_alpha_test(std::uint8_t(source>>24),test);
        const auto fail_action=std::uint32_t((test>>12)&3);
        if(!alpha_pass&&(fail_action==0||fail_action==2))return false; // KEEP / ZB_ONLY
        if(!gs_destination_alpha_test((destination&(1u<<31))!=0,psm,test))return false;
        const auto effective_primitive=primitive_state==0xffff?std::uint16_t(value[0]):primitive_state;
        const auto blend=(effective_primitive&(1u<<6))!=0 && (!(value[0x49]&1)||(source&0x80000000u)!=0);
        const auto color=blend?gs_alpha_blend(source,destination,value[0x42+context]):
            GsBlendResult{std::int32_t(source&0xff),std::int32_t((source>>8)&0xff),
                          std::int32_t((source>>16)&0xff),std::uint8_t(source>>24)};
        // RGB_ONLY preserves A only for RGBA32.  In RGB24/RGBA16 the manual
        // defines this mode as FB_ONLY, so do not accidentally preserve the
        // RGB24 padding byte or RGBA16's alpha bit.
        write_frame_pixel(context,x,y,color,!alpha_pass&&fail_action==3&&psm==0?0xff000000u:0u);
        return true;
    }

    // Normal primitive pixel path through TEST, PSMCT32/24/16, and PSMZ32/24/16.
    // A failed alpha test still takes the later destination-alpha and depth
    // tests; AFAIL then independently enables the documented frame/depth parts.
    bool draw_depth_frame_pixel(unsigned context,std::uint32_t x,std::uint32_t y,
                                std::uint32_t source,std::uint32_t z,std::uint16_t primitive_state=0xffff) {
        if(context>1)throw std::runtime_error("invalid GS frame context");
        // SCANMSK applies to primitive rasterization only (rather than host/local
        // buffer transfers).  Its row parity is the final framebuffer Y coordinate.
        const auto scanmask=std::uint32_t(value[0x22]&3);
        if((scanmask==2&&(y&1)==0)||(scanmask==3&&(y&1)!=0))return false;
        const auto test=value[0x47+context];
        const auto alpha_pass=gs_alpha_test(std::uint8_t(source>>24),test);
        const auto fail_action=std::uint32_t((test>>12)&3);
        const auto frame=value[0x4c+context];
        const auto psm=std::uint32_t((frame>>24)&0x3f);
        if(psm!=0&&psm!=1&&psm!=2&&psm!=10)throw std::runtime_error("unimplemented GS frame-buffer pixel format");
        const auto width=std::uint32_t((frame>>16)&0x3f)*64;
        if(!width)throw std::runtime_error("invalid GS frame-buffer width");
        if(vram.size()!=1024*1024)throw std::runtime_error("GS local memory is not initialized");
        const auto base=std::uint32_t(frame&0x1ff)*2048;
        const auto expand=[](std::uint32_t channel) {return (channel<<3)|(channel>>2);};
        const auto destination=[&] {
            if(psm==2||psm==10) {
                const auto packed=psm==2?pixel16(base,width,x,y):pixel16s(base,width,x,y);
                return expand(packed&31)|(expand((packed>>5)&31)<<8)|(expand((packed>>10)&31)<<16)|
                       ((packed&0x8000)?0x80000000u:0u);
            }
            const auto stored=vram[psmct32_word(base,width,x,y)];
            return psm==1?(stored&0xffffffu)|0x80000000u:stored;
        }();
        if(!gs_destination_alpha_test((destination&(1u<<31))!=0,psm,test))return false;
        if(!gs_depth_test(z,read_z_pixel(context,x,y),test))return false;
        if(!alpha_pass) {
            if(fail_action==0)return false; // KEEP
            if(fail_action==2) { // ZB_ONLY
                write_z_pixel(context,x,y,z);
                return true;
            }
            // FB_ONLY and RGBA32 RGB_ONLY flow through the frame writer,
            // which applies their AFAIL mask after blending and conversion.
            return draw_frame_pixel(context,x,y,source,primitive_state);
        }
        if(!draw_frame_pixel(context,x,y,source,primitive_state))throw std::runtime_error("GS pixel stage rejected after passed tests");
        write_z_pixel(context,x,y,z);
        return true;
    }

    // Rasterize the GS point primitive through the checked normal pixel path.
    // The GS point rule selects the nearest window-coordinate pixel. Because a
    // point has one vertex, STQ resolves directly from that vertex without an
    // interpolation step; multi-vertex perspective interpolation stays separate.
    bool rasterize_point(const GsDraw& draw) {
        if(draw.primitive!=0||draw.count!=1)throw std::runtime_error("invalid GS point draw");
        const auto context=unsigned((draw.prim_state>>9)&1);
        const auto vertex=draw.vertices[0];
        const auto nearest=[](std::int32_t fixed) {
            return fixed>=0?(fixed+8)/16:-((-fixed+8)/16);
        };
        const auto x=nearest(std::int32_t(vertex.x)-std::int32_t(draw.xyoffset&0xffff));
        const auto y=nearest(std::int32_t(vertex.y)-std::int32_t((draw.xyoffset>>32)&0xffff));
        const auto scissor=value[0x40+context];
        const auto x0=std::int32_t(scissor&0x7ff),x1=std::int32_t((scissor>>16)&0x7ff);
        const auto y0=std::int32_t((scissor>>32)&0x7ff),y1=std::int32_t((scissor>>48)&0x7ff);
        if(x<x0||x>x1||y<y0||y>y1)return false;
        std::uint32_t source=vertex.rgba;
        if(draw.prim_state&(1u<<4)) {
            std::int32_t u,v;
            if(draw.prim_state&(1u<<8)) {
                u=std::int32_t((vertex.uv&0x3fff)>>4);v=std::int32_t(((vertex.uv>>16)&0x3fff)>>4);
            } else {
                const auto tex0=value[6+context];
                u=gs_stq_texel_coordinate(std::uint32_t(vertex.st),vertex.q,1u<<((tex0>>26)&0xf));
                v=gs_stq_texel_coordinate(std::uint32_t(vertex.st>>32),vertex.q,1u<<((tex0>>30)&0xf));
            }
            source=shade_point_tex0(context,u,v,vertex.rgba);
        }
        if(draw.prim_state&(1u<<5))source=gs_apply_fog(source,std::uint8_t(vertex.fog),std::uint32_t(value[0x3d]));
        return draw_depth_frame_pixel(context,std::uint32_t(x),std::uint32_t(y),source,vertex.z,draw.prim_state);
    }

    // A sprite is an axis-aligned rectangle: its top/left edges are included
    // and bottom/right edges excluded. Its second vertex supplies Z. Keep its
    // FST sprites linearly interpolate their 10.4 UV endpoints along the two
    // rectangle axes. STQ sprites interpolate S/T/Q on those axes and resolve
    // the perspective ratio exactly without host floating point.
    bool rasterize_sprite(const GsDraw& draw) {
        if(draw.primitive!=6||draw.count!=2)throw std::runtime_error("invalid GS sprite draw");
        const auto textured=(draw.prim_state&(1u<<4))!=0;
        const auto& first=draw.vertices[0];
        const auto& second=draw.vertices[1];
        const auto fst=(draw.prim_state&(1u<<8))!=0;
        // Sprite shading is fixed flat and antialiasing is fixed off; IIP and
        // AA1 therefore do not override the drawing-kick vertex's attributes.
        const auto context=unsigned((draw.prim_state>>9)&1);
        const auto offset=draw.xyoffset;
        const auto ceil_fixed=[](std::int32_t value) {
            return value>=0 ? (value+15)/16 : -((-value)/16);
        };
        const auto ax_fixed=std::int32_t(first.x)-std::int32_t(offset&0xffff);
        const auto ay_fixed=std::int32_t(first.y)-std::int32_t((offset>>32)&0xffff);
        const auto bx_fixed=std::int32_t(second.x)-std::int32_t(offset&0xffff);
        const auto by_fixed=std::int32_t(second.y)-std::int32_t((offset>>32)&0xffff);
        const auto ax=ceil_fixed(ax_fixed),ay=ceil_fixed(ay_fixed);
        const auto bx=ceil_fixed(bx_fixed),by=ceil_fixed(by_fixed);
        const auto left=ax<bx?ax:bx, right=ax<bx?bx:ax;
        const auto top=ay<by?ay:by, bottom=ay<by?by:ay;
        const auto scissor_value=value[0x40+context];
        const auto x0=std::int32_t(scissor_value&0x7ff), x1=std::int32_t((scissor_value>>16)&0x7ff);
        const auto y0=std::int32_t((scissor_value>>32)&0x7ff), y1=std::int32_t((scissor_value>>48)&0x7ff);
        const auto u0=std::int64_t(first.uv&0x3fff),v0=std::int64_t((first.uv>>16)&0x3fff);
        const auto u1=std::int64_t(second.uv&0x3fff),v1=std::int64_t((second.uv>>16)&0x3fff);
        const auto interpolate=[](std::int64_t first_value,std::int64_t second_value,
                                  std::int64_t first_position,std::int64_t second_position,
                                  std::int64_t sample) {
            if(first_position==second_position)return first_value;
            return first_value+(sample-first_position)*(second_value-first_value)/(second_position-first_position);
        };
        const auto first_x=std::max(left,x0),last_x=std::min(right,x1+1);
        const auto first_y=std::max(top,y0),last_y=std::min(bottom,y1+1);
        if(first_x>=last_x||first_y>=last_y)return false;
        // Fixed UV sprite coordinates are separable: U depends only on X and
        // V only on Y. Reuse the exact integer interpolation without changing
        // pixel order (which matters for overlapping texture/frame storage).
        std::array<std::int32_t,2048> fixed_u{};
        if(textured&&fst)for(auto x=first_x;x<last_x;++x)
            fixed_u[unsigned(x-first_x)]=std::int32_t(interpolate(u0,u1,ax_fixed,bx_fixed,std::int64_t(x)*16)>>4);
        // Reduce the existing pixel pipeline only when its tests always pass,
        // depth is masked and blending is disabled. RGB24 preserves the high
        // byte and ignores FBA. Keep live texture reads and row-major writes.
        const auto frame=value[0x4c+context],test=value[0x47+context],zbuf=value[0x4e+context];
        const auto frame_width=std::uint32_t((frame>>16)&0x3f)*64;
        const auto zpsm=unsigned((zbuf>>24)&15);
        const bool direct_rgb24=((frame>>24)&63)==1 && frame_width &&
            vram.size()==1024*1024 && !(draw.prim_state&(1u<<6)) &&
            !(value[0x22]&3) && !(test&0x4001) && (test&0x70000)==0x30000 &&
            (zbuf&(1ull<<32)) && (zpsm==0||zpsm==1||zpsm==2||zpsm==10);
        const auto frame_base=std::uint32_t(frame&511)*2048;
        const auto frame_mask=std::uint32_t(frame>>32)|0xff000000u;
        bool drawn=false;
        for(auto y=first_y;y<last_y;++y) {
            const auto fixed_v=textured&&fst?std::int32_t(interpolate(v0,v1,ay_fixed,by_fixed,std::int64_t(y)*16)>>4):0;
            for(auto x=first_x;x<last_x;++x) {
                auto source=second.rgba;
                if(textured) {
                    std::int32_t u,v;
                    if(fst) {
                        u=fixed_u[unsigned(x-first_x)];v=fixed_v;
                    } else {
                        const auto weights=[](std::int64_t start,std::int64_t end,std::int64_t sample) {
                            if(end>start)return std::pair<std::uint64_t,std::uint64_t>{
                                std::uint64_t(end-sample),std::uint64_t(sample-start)};
                            return std::pair<std::uint64_t,std::uint64_t>{
                                std::uint64_t(sample-end),std::uint64_t(start-sample)};
                        };
                        const auto wx=weights(ax_fixed,bx_fixed,std::int64_t(x)*16);
                        const auto wy=weights(ay_fixed,by_fixed,std::int64_t(y)*16);
                        const auto tex0=value[6+context];
                        u=gs_stq_perspective_texel_coordinate(
                            {std::uint32_t(first.st),std::uint32_t(second.st),0},
                            {first.q,second.q,0},{std::int64_t(wx.first),std::int64_t(wx.second),0},1u<<((tex0>>26)&0xf));
                        v=gs_stq_perspective_texel_coordinate(
                            {std::uint32_t(first.st>>32),std::uint32_t(second.st>>32),0},
                            {first.q,second.q,0},{std::int64_t(wy.first),std::int64_t(wy.second),0},1u<<((tex0>>30)&0xf));
                    }
                    source=shade_point_tex0(context,u,v,source);
                }
                if(draw.prim_state&(1u<<5))source=gs_apply_fog(source,std::uint8_t(second.fog),std::uint32_t(value[0x3d]));
                if(direct_rgb24) {
                    auto& destination=vram[psmct32_word(frame_base,frame_width,std::uint32_t(x),std::uint32_t(y))];
                    destination=(destination&frame_mask)|(source&~frame_mask);drawn=true;
                } else drawn=draw_depth_frame_pixel(context,std::uint32_t(x),std::uint32_t(y),source,second.z,draw.prim_state)||drawn;
            }
        }
        return drawn;
    }

    // The GS line rule is the half-open segment through the diamond assigned
    // to each pixel: the start is included and the endpoint is excluded.  This
    // implements the documented coverage geometry before the ordinary scissor
    // and pixel stages. Rational interval clipping avoids host floating point;
    // the dominant-axis DDA supplies every varying attribute.
    bool rasterize_line(const GsDraw& draw) {
        if((draw.primitive!=1&&draw.primitive!=2)||draw.count!=2)
            throw std::runtime_error("invalid GS line draw");
        if(draw.prim_state&(1u<<7))throw std::runtime_error("unimplemented GS line antialiasing");
        const auto& first=draw.vertices[0];
        const auto& second=draw.vertices[1];
        const auto textured=(draw.prim_state&(1u<<4))!=0;
        const auto fst=(draw.prim_state&(1u<<8))!=0;
        const auto gouraud=(draw.prim_state&(1u<<3))!=0;
        const auto context=unsigned((draw.prim_state>>9)&1);
        const auto offset=draw.xyoffset;
        const auto ax=std::int64_t(first.x)-std::int64_t(offset&0xffff);
        const auto ay=std::int64_t(first.y)-std::int64_t((offset>>32)&0xffff);
        const auto bx=std::int64_t(second.x)-std::int64_t(offset&0xffff);
        const auto by=std::int64_t(second.y)-std::int64_t((offset>>32)&0xffff);
        if(ax==bx&&ay==by)return false;
        const auto dx=bx-ax,dy=by-ay;
        const auto magnitude=[](std::int64_t value){return value<0?-value:value;};
        const bool x_major=magnitude(dx)>=magnitude(dy);
        const auto floor_fixed=[](std::int64_t value) {return value>=0?value/16:-((-value+15)/16);};
        const auto ceil_fixed=[](std::int64_t value) {return value>=0?(value+15)/16:-((-value)/16);};
        const auto min=[](std::int64_t a,std::int64_t b) {return a<b?a:b;};
        const auto max=[](std::int64_t a,std::int64_t b) {return a>b?a:b;};
        // Transform |x-cx|+|y-cy| <= 8 into two axis-aligned intervals. A
        // pixel is drawn only when the segment exits its diamond before t=1;
        // merely ending inside the final diamond must not paint that endpoint
        // pixel, and leaves it for a possible continuation segment.
        const auto crosses_diamond=[&](std::int64_t cx,std::int64_t cy) {
            struct Fraction {std::int64_t numerator=0,denominator=1;};
            const auto less=[](Fraction a,Fraction b) {
                return a.numerator*b.denominator<b.numerator*a.denominator;
            };
            const auto less_equal=[&](Fraction a,Fraction b) {return !less(b,a);};
            Fraction low{},high{};bool has_low=false,has_high=false,valid=true;
            const auto clip=[&](std::int64_t initial,std::int64_t delta,std::int64_t center) {
                const auto lower=center-8,upper=center+8;
                if(!delta)return initial>=lower&&initial<=upper;
                const auto denominator=delta<0?-delta:delta;
                Fraction a{lower-initial,denominator},b{upper-initial,denominator};
                if(delta<0) {a.numerator=-a.numerator;b.numerator=-b.numerator;}
                if(less(b,a)) {const auto swap=a;a=b;b=swap;}
                if(!has_low||less(low,a)) {low=a;has_low=true;}
                if(!has_high||less(b,high)) {high=b;has_high=true;}
                valid=!has_low||!has_high||less_equal(low,high);
                return valid;
            };
            const auto ux=ax+ay,uy=bx+by,vx=ax-ay,vy=bx-by;
            if(!clip(ux,uy-ux,cx+cy)||!clip(vx,vy-vx,cx-cy)||!valid||!has_high)return false;
            const Fraction zero{0,1},one{1,1};
            return less_equal(zero,high)&&(!has_low||less(low,one))&&less(high,one);
        };
        const auto scissor=value[0x40+context];
        const auto x0=std::int64_t(scissor&0x7ff),x1=std::int64_t((scissor>>16)&0x7ff);
        const auto y0=std::int64_t((scissor>>32)&0x7ff),y1=std::int64_t((scissor>>48)&0x7ff);
        const auto left=max(x0,floor_fixed(min(ax,bx)-8)),right=min(x1,ceil_fixed(max(ax,bx)+8));
        const auto top=max(y0,floor_fixed(min(ay,by)-8)),bottom=min(y1,ceil_fixed(max(ay,by)+8));
        bool drawn=false;
        for(auto y=top;y<=bottom;++y)for(auto x=left;x<=right;++x)
            if(crosses_diamond(x*16,y*16)) {
                const auto position=x_major?std::int64_t(x)*16:std::int64_t(y)*16;
                const auto start=x_major?ax:ay,end=x_major?bx:by;
                const auto interpolate=[&](std::int64_t first_value,std::int64_t second_value) {
                    return first_value+(position-start)*(second_value-first_value)/(end-start);
                };
                auto source=second.rgba; // Flat shading takes the drawing-kick vertex.
                if(gouraud) {
                    source=0;
                    for(unsigned shift=0;shift<32;shift+=8)
                        source|=std::uint32_t(interpolate((first.rgba>>shift)&0xff,(second.rgba>>shift)&0xff))<<shift;
                }
                if(textured) {
                    std::int32_t u,v;
                    if(fst) {
                        u=std::int32_t(interpolate(first.uv&0x3fff,second.uv&0x3fff)>>4);
                        v=std::int32_t(interpolate((first.uv>>16)&0x3fff,(second.uv>>16)&0x3fff)>>4);
                    } else {
                        auto first_weight=end-position,second_weight=position-start;
                        if(end<start) {first_weight=-first_weight;second_weight=-second_weight;}
                        const auto tex0=value[6+context];
                        u=gs_stq_perspective_texel_coordinate(
                            {std::uint32_t(first.st),std::uint32_t(second.st),0},{first.q,second.q,0},
                            {first_weight,second_weight,0},1u<<((tex0>>26)&0xf));
                        v=gs_stq_perspective_texel_coordinate(
                            {std::uint32_t(first.st>>32),std::uint32_t(second.st>>32),0},{first.q,second.q,0},
                            {first_weight,second_weight,0},1u<<((tex0>>30)&0xf));
                    }
                    source=shade_point_tex0(context,u,v,source);
                }
                if(draw.prim_state&(1u<<5))
                    source=gs_apply_fog(source,std::uint8_t(interpolate(first.fog,second.fog)),std::uint32_t(value[0x3d]));
                const auto z=std::uint32_t(interpolate(first.z,second.z));
                drawn=draw_depth_frame_pixel(context,std::uint32_t(x),std::uint32_t(y),source,z,draw.prim_state)||drawn;
            }
        return drawn;
    }

    // The GS triangle DDA includes a pixel center on top/left edges and
    // excludes it on bottom/right edges. Integer edge weights drive FST UV,
    // Gouraud RGBA, and fog interpolation from the same covered sample.
    // Varying depth and perspective STQ use these same exact weights.
    bool rasterize_triangle(const GsDraw& draw) {
        if((draw.primitive<3||draw.primitive>5)||draw.count!=3)throw std::runtime_error("invalid GS triangle draw");
        const auto textured=(draw.prim_state&(1u<<4))!=0;
        const auto fst=(draw.prim_state&(1u<<8))!=0;
        if(draw.prim_state&(1u<<7))throw std::runtime_error("prohibited GS triangle antialiasing");
        auto a=draw.vertices[0],b=draw.vertices[1],c=draw.vertices[2];
        const auto flat_fragment=c.rgba;
        const auto gouraud=(draw.prim_state&(1u<<3))!=0;
        const auto offset=draw.xyoffset;
        struct Point { std::int64_t x,y; };
        const auto point=[&](const GsVertex& vertex) {
            return Point{std::int64_t(vertex.x)-std::int64_t(offset&0xffff),
                         std::int64_t(vertex.y)-std::int64_t((offset>>32)&0xffff)};
        };
        auto pa=point(a),pb=point(b),pc=point(c);
        const auto edge=[](Point first,Point second,Point sample) {
            return (second.x-first.x)*(sample.y-first.y)-(second.y-first.y)*(sample.x-first.x);
        };
        auto area=edge(pa,pb,pc);
        if(!area)return false;
        if(area<0) {const auto point_swap=pb;pb=pc;pc=point_swap;const auto vertex_swap=b;b=c;c=vertex_swap;area=-area;}
        const auto top_left=[](Point first,Point second) {
            const auto dx=second.x-first.x,dy=second.y-first.y;
            return dy<0 || (dy==0&&dx>0);
        };
        const auto covered=[&](Point first,Point second,Point sample) {
            const auto value=edge(first,second,sample);
            return value>0 || (value==0&&top_left(first,second));
        };
        const auto floor_fixed=[](std::int64_t value) {return value>=0?value/16:-((-value+15)/16);};
        const auto ceil_fixed=[](std::int64_t value) {return value>=0?(value+15)/16:-((-value)/16);};
        const auto min3=[](std::int64_t x,std::int64_t y,std::int64_t z) {return x<y?(x<z?x:z):(y<z?y:z);};
        const auto max3=[](std::int64_t x,std::int64_t y,std::int64_t z) {return x>y?(x>z?x:z):(y>z?y:z);};
        const auto left=ceil_fixed(min3(pa.x,pb.x,pc.x)),right=floor_fixed(max3(pa.x,pb.x,pc.x));
        const auto top=ceil_fixed(min3(pa.y,pb.y,pc.y)),bottom=floor_fixed(max3(pa.y,pb.y,pc.y));
        const auto context=unsigned((draw.prim_state>>9)&1);
        const auto scissor=value[0x40+context];
        const auto x0=std::int64_t(scissor&0x7ff),x1=std::int64_t((scissor>>16)&0x7ff);
        const auto y0=std::int64_t((scissor>>32)&0x7ff),y1=std::int64_t((scissor>>48)&0x7ff);
        // Flat shading takes the RGBAQ state immediately preceding this draw's
        // XYZ2 kick, which is the final triangle vertex captured here.
        bool drawn=false;
        for(auto y=top>y0?top:y0;y<=bottom&&y<=y1;++y) for(auto x=left>x0?left:x0;x<=right&&x<=x1;++x) {
            const Point sample{x*16,y*16};
            if(covered(pa,pb,sample)&&covered(pb,pc,sample)&&covered(pc,pa,sample)) {
                auto source=flat_fragment;
                const auto wa=edge(pb,pc,sample),wb=edge(pc,pa,sample),wc=edge(pa,pb,sample);
                const auto interpolate=[&](std::int64_t av,std::int64_t bv,std::int64_t cv) {
                    return (wa*av+wb*bv+wc*cv)/area;
                };
                // GS X/Y fields are 16-bit, so triangle area is at most
                // 65535^2. Covered nonnegative weights sum to that area;
                // therefore the weighted uint32 sum fits exactly in uint64.
                const auto interpolate_u32=[&](std::uint32_t av,std::uint32_t bv,std::uint32_t cv) {
                    const auto sum=std::uint64_t(wa)*av+std::uint64_t(wb)*bv+std::uint64_t(wc)*cv;
                    return std::uint32_t(sum/std::uint64_t(area));
                };
                if(gouraud) {
                    source=0;
                    for(unsigned shift=0;shift<32;shift+=8)
                        source|=std::uint32_t(interpolate((a.rgba>>shift)&0xff,(b.rgba>>shift)&0xff,
                                                         (c.rgba>>shift)&0xff))<<shift;
                } else source=flat_fragment;
                if(textured) {
                    std::int32_t u,v;
                    if(fst) {
                        const auto component=[&](unsigned shift) {
                            const auto av=std::int64_t((a.uv>>shift)&0x3fff),bv=std::int64_t((b.uv>>shift)&0x3fff);
                            const auto cv=std::int64_t((c.uv>>shift)&0x3fff);
                            return interpolate(av,bv,cv);
                        };
                        u=std::int32_t(component(0)>>4);v=std::int32_t(component(16)>>4);
                    } else {
                        const auto tex0=value[6+context];
                        u=gs_stq_perspective_texel_coordinate(
                            {std::uint32_t(a.st),std::uint32_t(b.st),std::uint32_t(c.st)},
                            {a.q,b.q,c.q},{wa,wb,wc},1u<<((tex0>>26)&0xf));
                        v=gs_stq_perspective_texel_coordinate(
                            {std::uint32_t(a.st>>32),std::uint32_t(b.st>>32),std::uint32_t(c.st>>32)},
                            {a.q,b.q,c.q},{wa,wb,wc},1u<<((tex0>>30)&0xf));
                    }
                    source=shade_point_tex0(context,u,v,source);
                }
                if(draw.prim_state&(1u<<5))
                    source=gs_apply_fog(source,std::uint8_t(interpolate(a.fog,b.fog,c.fog)),std::uint32_t(value[0x3d]));
                const auto z=interpolate_u32(a.z,b.z,c.z);
                drawn=draw_depth_frame_pixel(context,std::uint32_t(x),std::uint32_t(y),source,z,draw.prim_state)||drawn;
            }
        }
        return drawn;
    }

    bool rasterize_draw(const GsDraw& draw) {
        const auto dispatch=[&] {
            if(draw.primitive==0)return rasterize_point(draw);
            if(draw.primitive==1||draw.primitive==2)return rasterize_line(draw);
            if(draw.primitive>=3&&draw.primitive<=5)return rasterize_triangle(draw);
            if(draw.primitive==6)return rasterize_sprite(draw);
            throw std::runtime_error("unimplemented GS primitive rasterizer");
        };
        if(!draw.environment_captured)return dispatch();
        const auto live=value;
        value=draw.environment;
        try {const auto result=dispatch();value=live;return result;}
        catch(...) {value=live;throw;}
    }

    std::size_t rasterize_pending_draws() {
        std::size_t completed=0;
        for(auto pending=pending_draw_index();pending<draws.size();++pending) {
            // Advance only after the draw returns; an explicit unsupported
            // rasterizer fault leaves the queue position available to inspect.
            rasterize_draw(draws[pending]);
            ++rasterized_draw_count;
            ++completed;
        }
        return completed;
    }

    void begin_transfer(std::uint64_t direction) {
        const auto xdir=unsigned(direction&3);
        host_transfer_completed=false;
        if(xdir==3) { // Transmission deactivated; an in-flight host upload ends.
            gif_to_vram_active=false;transfer_remaining=0;return;
        }
        if(xdir==1)throw std::runtime_error("unsupported GS local-to-host transfer");
        const auto bitblt = value[0x50], position = value[0x51], region = value[0x52];
        const auto format = (bitblt >> 56) & 0x3f;
        if (xdir==0 && format != 0 && format != 1 && format != 2 && format != 10 &&
            format != 0x30 && format != 0x31 && format != 0x32 && format != 0x3a &&
            format != 0x13 && format != 0x14 && format != 0x1b && format != 0x24 && format != 0x2c)
            throw std::runtime_error("unsupported GS destination pixel format");
        const auto width = std::uint32_t(region & 0xfff), height = std::uint32_t((region >> 32) & 0xfff);
        if (!width || !height) throw std::runtime_error("invalid GS transfer dimensions");
        if (std::uint64_t(width) * height > 1024ull * 1024ull)
            throw std::runtime_error("GS transfer exceeds local-memory capacity");
        if(xdir==0) {
            const auto destination_width=std::uint32_t((bitblt>>48)&0x3f)*64;
            const auto destination_x=std::uint32_t((position>>32)&0x7ff);
            validate_transfer_buffer_width(unsigned(format),destination_width);
            const auto depth=transfer_pixel_depth(unsigned(format));
            const auto width_alignment=depth>=32?2u:depth==24?8u:depth==16?4u:8u;
            if(width%width_alignment||destination_x%transfer_x_alignment(unsigned(format)))
                throw std::runtime_error("invalid GS host-to-local transfer alignment");
        }
        // Drawing and host-to-local transfers share GS local memory.  Commit
        // every earlier primitive before IMAGE payload can modify its frame,
        // depth, texture, or CLUT storage; later primitives remain deferred.
        rasterize_pending_draws();
        if(xdir==2) {
            const auto source_format=unsigned((bitblt>>24)&0x3f);
            const auto depth=transfer_pixel_depth(source_format);
            if(transfer_pixel_depth(unsigned(format))!=depth)
                throw std::runtime_error("GS local-to-local pixel depths differ");
            const auto source_width=std::uint32_t((bitblt>>16)&0x3f)*64;
            const auto destination_width=std::uint32_t((bitblt>>48)&0x3f)*64;
            validate_transfer_buffer_width(source_format,source_width);
            validate_transfer_buffer_width(unsigned(format),destination_width);
            const auto width_alignment=depth>=32?2u:depth==24?8u:depth==16?4u:8u;
            if(width%width_alignment)throw std::runtime_error("invalid GS local-to-local transfer width alignment");
            ensure_vram();
            const auto source_base=std::uint32_t(bitblt&0x3fff)*64;
            const auto destination_base=std::uint32_t((bitblt>>32)&0x3fff)*64;
            const auto source_x=std::uint32_t(position&0x7ff),source_y=std::uint32_t((position>>16)&0x7ff);
            const auto destination_x=std::uint32_t((position>>32)&0x7ff),destination_y=std::uint32_t((position>>48)&0x7ff);
            if(source_x%transfer_x_alignment(source_format)||
               destination_x%transfer_x_alignment(unsigned(format)))
                throw std::runtime_error("invalid GS local-to-local start X alignment");
            const auto order=unsigned((position>>59)&3);
            for(std::uint32_t row=0;row<height;++row)for(std::uint32_t column=0;column<width;++column) {
                const auto dx=(order&2)?width-1-column:column;
                const auto dy=(order&1)?height-1-row:row;
                const auto sx=(source_x+dx)&0x7ff,sy=(source_y+dy)&0x7ff;
                const auto tx=(destination_x+dx)&0x7ff,ty=(destination_y+dy)&0x7ff;
                const auto pixel=read_transfer_pixel(source_format,source_base,source_width,sx,sy);
                write_transfer_pixel(unsigned(format),destination_base,destination_width,tx,ty,pixel);
            }
            gif_to_vram_active=false;transfer_remaining=0;return;
        }
        transfer_x = std::uint32_t((position >> 32) & 0x7ff);
        transfer_y = std::uint32_t((position >> 48) & 0x7ff);
        transfer_width = width;
        transfer_remaining = width * height;
        ensure_vram();
        gif_to_vram_active = true;
    }

    void write_hwreg(std::uint64_t data) {
        if(completed_host_payload())return;
        if (!gif_to_vram_active) throw std::runtime_error("GS HWREG write without GIF-to-VRAM transfer");
        const auto bitblt = value[0x50];
        const auto base_words = std::uint32_t((bitblt >> 32) & 0x3fffu) * 64;
        const auto buffer_width = std::uint32_t((bitblt >> 48) & 0x3fu) * 64;
        const auto format = unsigned((bitblt >> 56) & 0x3f);
        if (format != 0 && format != 2 && format != 10 && format!=0x30 && format!=0x32 && format!=0x3a)
            throw std::runtime_error("GS HWREG format requires packed-image handling");
        const unsigned bits = transfer_pixel_depth(format);
        for (unsigned half = 0; half != 64 / bits; ++half) {
            // The last HWREG may contain padding beyond an odd-sized region.
            if (!transfer_remaining) return;
            const auto written_pixels = std::uint64_t(value[0x52] & 0xfff) *
                                        std::uint64_t((value[0x52] >> 32) & 0xfff) - transfer_remaining;
            const auto x = transfer_x + std::uint32_t(written_pixels % transfer_width);
            const auto y = transfer_y + std::uint32_t(written_pixels / transfer_width);
            write_transfer_pixel(format,base_words,buffer_width,x,y,std::uint32_t(data>>(half*bits)));
            if (--transfer_remaining == 0) {
                gif_to_vram_active=false;host_transfer_completed=true;completed_transfer_format=format;
            }
        }
    }

    bool completed_host_payload() const {
        // GS fixed-size rectangle, measured synthetic exact/double upload
        // profile (ORACLE.md). Never accept data before a valid completion or
        // after cancellation. Unmeasured pixel formats remain explicit faults.
        const auto format=unsigned((value[0x50]>>56)&0x3f);
        return host_transfer_completed && !gif_to_vram_active && !transfer_remaining &&
            format==completed_transfer_format && (format==0||format==2||format==0x13||format==0x14);
    }
    void write_image_qword(std::uint64_t low, std::uint64_t high) {
        if(completed_host_payload())return;
        const auto format = unsigned((value[0x50] >> 56) & 0x3f);
        if (format != 1 && format != 0x31 && format != 0x13 && format != 0x14 && format != 0x1b && format != 0x24 && format != 0x2c) {
            write_hwreg(low);
            if (gif_to_vram_active) write_hwreg(high);
            return;
        }
        if (!gif_to_vram_active) throw std::runtime_error("GS IMAGE data without GIF-to-VRAM transfer");
        std::array<std::uint8_t, 16> bytes{};
        for (unsigned i = 0; i != 8; ++i) {
            bytes[i] = std::uint8_t(low >> (i * 8));
            bytes[i + 8] = std::uint8_t(high >> (i * 8));
        }
        const auto bitblt = value[0x50];
        const auto base_words = std::uint32_t((bitblt >> 32) & 0x3fffu) * 64;
        const auto buffer_width = std::uint32_t((bitblt >> 48) & 0x3fu) * 64;
        const unsigned count = (format == 1||format==0x31) ? 5 : (format == 0x13 || format == 0x1b) ? 16 : 32;
        for (unsigned pixel = 0; pixel != count && transfer_remaining; ++pixel) {
            const auto written_pixels = std::uint64_t(value[0x52] & 0xfff) *
                                        std::uint64_t((value[0x52] >> 32) & 0xfff) - transfer_remaining;
            const auto x = transfer_x + std::uint32_t(written_pixels % transfer_width);
            const auto y = transfer_y + std::uint32_t(written_pixels / transfer_width);
            if (format == 1||format==0x31) {
                const auto rgb = std::uint32_t(bytes[pixel * 3]) |
                                 (std::uint32_t(bytes[pixel * 3 + 1]) << 8) |
                                 (std::uint32_t(bytes[pixel * 3 + 2]) << 16);
                write_transfer_pixel(format,base_words,buffer_width,x,y,rgb);
            } else if (format == 0x13) {
                auto& word = vram[psmt8_word(base_words, buffer_width, x, y)];
                const auto lane = ((y & 2) >> 1) | ((x & 8) >> 2);
                word = (word & ~(0xffu << (lane * 8))) | (std::uint32_t(bytes[pixel]) << (lane * 8));
            } else if (format == 0x1b) {
                auto& word=vram[psmct32_word(base_words,buffer_width,x,y)];
                word=(word&0x00ffffffu)|(std::uint32_t(bytes[pixel])<<24);
            } else {
                const auto nibble = (bytes[pixel / 2] >> ((pixel & 1) * 4)) & 0xf;
                if(format==0x14) {
                    auto& word = vram[psmt4_word(base_words, buffer_width, x, y)];
                    const auto lane = ((y & 2) >> 1) | ((x & 24) >> 2);
                    word = (word & ~(0xfu << (lane * 4))) | (std::uint32_t(nibble) << (lane * 4));
                } else {
                    auto& word=vram[psmct32_word(base_words,buffer_width,x,y)];
                    const auto shift=format==0x24?24:28;
                    const auto mask=0xfu<<shift;
                    word=(word&~mask)|(std::uint32_t(nibble)<<shift);
                }
            }
            if (--transfer_remaining == 0) {
                gif_to_vram_active=false;host_transfer_completed=true;completed_transfer_format=format;
            }
        }
    }
};

// Commit only operations with defined register addressing at this stage.  A+D
// carries its GS address in bits 64-71 of a PACKED quadword. PRE carries a
// documented PRIM write. Other descriptors remain explicit boundaries until
// their vertex/register data formats are implemented.
inline void apply_gif_register_transfers(const GifPacket& packet, GsRegisterState& gs) {
    for (const auto& transfer : packet.transfers) {
        if(transfer.reset_q)gs.gif_q=0x3f800000;
        if (transfer.kind == GifTransferKind::Prim) {
            gs.write_ad(0, transfer.low);
        } else if (transfer.kind == GifTransferKind::Packed) {
            gs.write_packed(transfer.descriptor, transfer.low, transfer.high);
        } else if (transfer.kind == GifTransferKind::Reglist) {
            gs.write_reglist(transfer.descriptor, transfer.low);
        } else if (transfer.kind == GifTransferKind::Image) {
            gs.write_image_qword(transfer.low, transfer.high);
        } else {
            throw std::runtime_error("GIF transfer requires unimplemented GS descriptor handling");
        }
    }
}

// A GIF path accepts DMA qwords without assuming packet alignment. Complete
// packets are decoded and committed in order; a bounded suffix remains queued.
struct GifPath {
    std::vector<std::uint8_t> pending;
    std::uint32_t read_status(bool vif_path3_masked,bool reverse) const {
        // Complete packets are committed synchronously. A retained suffix is
        // not a hardware FIFO and has no independently modeled active path.
        if(!pending.empty())throw std::runtime_error("GIF STAT during incomplete packet requires path/FIFO modeling");
        return (vif_path3_masked?2u:0u)|(reverse?0x1000u:0u);
    }
    void write_control(std::uint32_t value) {
        if(value!=1)throw std::runtime_error("unsupported GIF CTRL stop/restart operation");
        // Reset the GIF transport/parser, not the receiving GS registers/VRAM.
        pending.clear();
    }
    static constexpr std::size_t max_pending = 8 * 1024 * 1024 + 16;

    void submit_qword(std::uint64_t low, std::uint64_t high, GsRegisterState& gs) {
        for (unsigned i = 0; i != 8; ++i) {
            pending.push_back(std::uint8_t(low >> (i * 8)));
        }
        for (unsigned i = 0; i != 8; ++i) {
            pending.push_back(std::uint8_t(high >> (i * 8)));
        }
        if (pending.size() > max_pending) throw std::runtime_error("GIF packet exceeds bounded path capacity");
        while (const auto packet_size = gif_packet_size(pending.data(), pending.size())) {
            const auto packet = decode_gif_packet(pending.data(), *packet_size);
            apply_gif_register_transfers(packet, gs);
            pending.erase(pending.begin(), pending.begin() + *packet_size);
        }
    }
};

} // namespace hg
