#pragma once

#include "hg/gs.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace hg {

enum class HostTopology : std::uint8_t { points, lines, triangles };
struct HostVertex {
    float x=0,y=0,z=0,r=0,g=0,b=0,a=0,u=0,v=0;
};
struct HostGeometry {
    HostTopology topology=HostTopology::points;
    bool textured=false,fixed_texture_coordinates=false,gouraud=false,alpha_blend=false;
    std::uint8_t context=0;
    std::vector<HostVertex> vertices;
};

// Preserve GS screen-space coordinates and draw order in a backend-neutral
// geometry record. The OpenGL adapter owns viewport/XYOFFSET/depth semantics;
// this stage deliberately does not invent a projection or rasterization rule.
inline HostVertex host_vertex(const GsVertex& source,std::uint64_t xyoffset) {
    const auto byte=[](std::uint32_t value,unsigned shift) {return float((value>>shift)&0xff)/255.0f;};
    const auto x=std::int32_t(source.x)-std::int32_t(xyoffset&0xffff);
    const auto y=std::int32_t(source.y)-std::int32_t((xyoffset>>32)&0xffff);
    return {float(x)/16.0f,float(y)/16.0f,float(source.z),
            byte(source.rgba,0),byte(source.rgba,8),byte(source.rgba,16),byte(source.rgba,24),
            float(source.uv&0x3fff)/16.0f,float((source.uv>>16)&0x3fff)/16.0f};
}
inline HostGeometry presentation_geometry(const GsDraw& draw) {
    const auto primitive=draw.primitive;
    HostGeometry result{};
    result.textured=(draw.prim_state&(1u<<4))!=0;
    result.fixed_texture_coordinates=(draw.prim_state&(1u<<8))!=0;
    result.gouraud=(draw.prim_state&(1u<<3))!=0;
    result.alpha_blend=(draw.prim_state&(1u<<6))!=0;
    result.context=std::uint8_t((draw.prim_state>>9)&1);
    const auto add=[&](const GsVertex& vertex) {result.vertices.push_back(host_vertex(vertex,draw.xyoffset));};
    if(primitive==0) {if(draw.count!=1)throw std::runtime_error("invalid GS point draw");result.topology=HostTopology::points;add(draw.vertices[0]);}
    else if(primitive==1||primitive==2) {if(draw.count!=2)throw std::runtime_error("invalid GS line draw");result.topology=HostTopology::lines;add(draw.vertices[0]);add(draw.vertices[1]);}
    else if(primitive==3||primitive==4||primitive==5) {if(draw.count!=3)throw std::runtime_error("invalid GS triangle draw");result.topology=HostTopology::triangles;add(draw.vertices[0]);add(draw.vertices[1]);add(draw.vertices[2]);}
    else if(primitive==6) {
        if(draw.count!=2)throw std::runtime_error("invalid GS sprite draw");result.topology=HostTopology::triangles;
        const auto a=draw.vertices[0],b=draw.vertices[1];
        GsVertex right_top=a,left_bottom=a;right_top.x=b.x;left_bottom.y=b.y;
        add(a);add(right_top);add(left_bottom);add(right_top);add(b);add(left_bottom);
    } else throw std::runtime_error("prohibited GS primitive topology");
    return result;
}

} // namespace hg
