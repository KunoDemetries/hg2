#pragma once
#include "hg/vif.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// Independent scalar/formula reference for one isolated UNPACK command. It
// deliberately retains division/remainder rather than sharing the candidate's
// loop-carried index calculation. No game input or emulator implementation.
namespace vif_unpack_regression {
inline std::uint32_t read32(const std::vector<std::uint8_t>& b,std::size_t at) {
    return std::uint32_t(b.at(at))|(std::uint32_t(b.at(at+1))<<8)|
           (std::uint32_t(b.at(at+2))<<16)|(std::uint32_t(b.at(at+3))<<24);
}
inline constexpr std::array<unsigned,16> field_bits={32,16,8,0,32,16,8,0,32,16,8,0,32,16,8,16};
inline constexpr std::array<unsigned,16> field_count={1,1,1,0,2,2,2,0,3,3,3,0,4,4,4,4};
inline void reference(hg::Vif1Path& v) {
    if(v.pending.size()>hg::Vif1Path::max_pending)throw std::runtime_error("VIF1 packet exceeds bounded path capacity");
    if(v.pending.size()<4)return;
    const auto code=read32(v.pending,0);
    if(code&0x80000000u)throw std::runtime_error("VIF1 interrupt-control VIFcode requires interrupt delivery");
    const unsigned format=(code>>24)&15u,bits=field_bits[format],fields=field_count[format];
    if(!bits)throw std::runtime_error("unsupported VIF1 UNPACK format");
    const bool masked=(code&0x10000000u)!=0,scalar=fields==1,packed=format==15;
    const auto imm=std::uint16_t(code);
    const std::size_t count=((code>>16)&255u)?((code>>16)&255u):256u;
    const std::size_t cl=(v.cycle&255u)?(v.cycle&255u):256u;
    const std::size_t wl=(v.cycle>>8)?(v.cycle>>8):256u;
    if(cl<wl&&!masked)throw std::runtime_error("VIF1 UNPACK fill-cycle requires an explicit mask");
    const std::size_t destination=std::uint32_t((imm&0x8000u?v.tops:0u)+(imm&1023u));
    std::size_t supplied=0;
    for(std::size_t n=0;n<count;++n)if(cl>=wl||n%wl<cl)++supplied;
    const std::size_t unit=packed?2:fields*bits/8;
    const std::size_t payload=(supplied*unit+3)&~std::size_t(3);
    if(v.pending.size()<4+payload)return;
    std::size_t data=4;
    for(std::size_t n=0;n<count;++n) {
        const std::size_t phase=n%wl;
        const bool has_input=cl>=wl||phase<cl;
        const auto vector=cl>=wl?destination+cl*(n/wl)+phase:destination+n;
        if(vector>=1024)throw std::runtime_error("VIF1 UNPACK exceeds VU1 data memory");
        const auto mask_row=std::min<std::size_t>(phase,3);
        std::uint16_t packed_value=0;
        if(has_input&&packed)packed_value=std::uint16_t(v.pending.at(data))|(std::uint16_t(v.pending.at(data+1))<<8);
        for(unsigned field=0;field<4;++field) {
            const unsigned selector=masked?unsigned((v.mask>>(mask_row*8+field*2))&3u):0u;
            if(selector==3)continue;
            const auto defined_bit=std::uint8_t(1u<<field);
            if(selector==1) {v.vu_mem[vector*4+field]=v.row[field];v.vu_mem_defined[vector]|=defined_bit;continue;}
            if(selector==2) {v.vu_mem[vector*4+field]=v.column[mask_row];v.vu_mem_defined[vector]|=defined_bit;continue;}
            if(!has_input)throw std::runtime_error("VIF1 UNPACK fill-cycle input selection without source remains unsupported");
            if(!scalar&&field>=fields) {v.vu_mem_defined[vector]&=std::uint8_t(~defined_bit);continue;}
            if(packed) {
                const auto component=field==3?unsigned(packed_value>>15):unsigned((packed_value>>(5*field))&31u);
                v.vu_mem[vector*4+field]=component<<(field==3?7:3);v.vu_mem_defined[vector]|=defined_bit;continue;
            }
            const std::size_t at=data+(scalar?0:field*bits/8);
            std::uint32_t value=0;
            for(unsigned byte=0;byte<bits/8;++byte)value|=std::uint32_t(v.pending.at(at+byte))<<(8*byte);
            if(bits!=32&&!(imm&0x4000u)) {
                const auto sign=std::uint32_t(1)<<(bits-1);
                value=(value^sign)-sign;
            }
            if(v.mode) {value+=v.row[field];if(v.mode==2)v.row[field]=value;}
            v.vu_mem[vector*4+field]=value;v.vu_mem_defined[vector]|=defined_bit;
        }
        if(has_input)data+=unit;
    }
    v.pending.erase(v.pending.begin(),v.pending.begin()+4+payload);
    v.pending_phase=(v.pending_phase+4+payload)%16;
}

inline int run() {
    std::uint64_t formula_cases=0;
    for(std::size_t cl=1;cl<=256;++cl)for(std::size_t wl=1;wl<=256;++wl) {
        std::size_t vector=0,phase=0;
        const auto gap=cl>=wl?cl-wl:0;
        for(std::size_t n=0;n<256;++n) {
            const auto expected=cl>=wl?cl*(n/wl)+n%wl:n;
            if(vector!=expected||phase!=n%wl) {std::cerr<<"UNPACK cursor algebra mismatch\n";return 1;}
            ++vector;if(++phase==wl){phase=0;vector+=gap;}
            ++formula_cases;
        }
    }
    auto initial=std::make_unique<hg::Vif1Path>();
    auto gs=std::make_unique<hg::GsRegisterState>();
    hg::GifPath gif;
    for(std::size_t n=0;n<initial->vu_mem.size();++n)initial->vu_mem[n]=0xa5a50000u^std::uint32_t(n*0x1020305u);
    for(std::size_t n=0;n<initial->vu_mem_defined.size();++n)initial->vu_mem_defined[n]=std::uint8_t(n&15u);
    for(std::size_t n=0;n<initial->vu_micro_mem.size();++n)initial->vu_micro_mem[n]=0x0123456789abcdefull+n;
    initial->row={0xffffffffu,0x7fffffffu,0x80000000u,0x12345678u};
    initial->column={0xabcdef01u,0x76543210u,0x01020304u,0xffffffffu};
    initial->vu1.issue_cycle=0x123456789abcdef0ull;initial->vu1.tpc=0x288;
    constexpr std::array<std::uint32_t,16> cycles={0x0000,0x0101,0x0104,0x0304,0x0207,0xff01,0x03ff,0xfffe,
                                                 0x04ff,0x0402,0x0503,0x0100,0x0001,0x0010,0xffff,0xffff0101};
    constexpr std::array<unsigned,8> counts={1,2,3,5,16,64,255,256};
    constexpr std::array<unsigned,6> destinations={0,1,31,768,1000,1023};
    constexpr std::array<std::uint32_t,7> masks={0,0xffffffffu,0x55555555u,0xaaaaaaaau,0xe4e4e4e4u,0x1b1b1b1bu,0x3972ac5eu};
    std::uint32_t seed=0x16af428bu;
    const auto random=[&](){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;};
    std::uint64_t cases=0,faults=0,incomplete=0;
    for(unsigned format=0;format<16;++format)for(const auto cycle:cycles)for(const auto count:counts)
    for(unsigned mode=0;mode<3;++mode)for(unsigned variant=0;variant<4;++variant) {
        hg::Vif1Path expected=*initial,actual=*initial;
        expected.cycle=cycle;expected.mode=mode;expected.mask=masks[cases%masks.size()];
        expected.pending_phase=unsigned(cases%16);
        const bool relative=(cases&4u)!=0,masked=(variant&1u)!=0;
        expected.tops=cases%13==0?0xfffffffeu:(cases&8u?64u:0u);
        const unsigned imm=destinations[(cases/4)%destinations.size()]|(relative?0x8000u:0u)|(variant&2u?0x4000u:0u);
        std::uint32_t code=((0x60u|format|(masked?0x10u:0u))<<24)|((count&255u)<<16)|imm;
        if(cases%127==0)code|=0x80000000u;
        const std::size_t cl=(cycle&255u)?(cycle&255u):256u,wl=(cycle>>8)?(cycle>>8):256u;
        std::size_t input=0;for(std::size_t n=0;n<count;++n)if(cl>=wl||n%wl<cl)++input;
        const std::size_t unit=format==15?2:field_bits[format]*field_count[format]/8;
        const std::size_t payload=(input*unit+3)&~std::size_t(3);
        expected.pending.resize(4+payload);
        for(unsigned n=0;n<4;++n)expected.pending[n]=std::uint8_t(code>>(8*n));
        for(std::size_t n=4;n<expected.pending.size();++n)expected.pending[n]=std::uint8_t(random());
        if(cases%11==0)expected.pending.resize(3);
        else if(cases%11==1&&payload)expected.pending.pop_back();
        else if(cases%11==2&&payload)expected.pending.resize(4);
        actual=expected;
        std::string ref_error,got_error;
        try {reference(expected);}catch(const std::exception& error){ref_error=error.what();}
        try {actual.process_pending(gif,*gs);}catch(const std::exception& error){got_error=error.what();}
        const bool equal=ref_error==got_error&&actual.vu_mem==expected.vu_mem&&actual.vu_mem_defined==expected.vu_mem_defined&&
            actual.row==expected.row&&actual.column==expected.column&&actual.pending==expected.pending&&
            actual.pending_phase==expected.pending_phase&&actual.vu_micro_mem==expected.vu_micro_mem&&
            actual.cycle==expected.cycle&&actual.mode==expected.mode&&actual.mask==expected.mask&&actual.tops==expected.tops&&
            actual.vu1.issue_cycle==expected.vu1.issue_cycle&&actual.vu1.tpc==expected.vu1.tpc&&gif.pending.empty();
        if(!equal) {
            std::cerr<<"UNPACK differential failure case="<<cases<<" format="<<format<<" cycle="<<cycle
                     <<" count="<<count<<" mode="<<mode<<" variant="<<variant<<" expected='"<<ref_error<<"' got='"<<got_error<<"'\n";
            return 1;
        }
        if(!ref_error.empty())++faults;else if(!expected.pending.empty())++incomplete;
        ++cases;
    }
    std::cout<<"VIF UNPACK: "<<formula_cases<<" exact cursor indices; "<<cases<<" independent state/fault fixtures ("
             <<faults<<" explicit faults, "<<incomplete<<" incomplete commands) passed\n";
    return 0;
}
}
