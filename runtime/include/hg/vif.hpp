#pragma once

#include "hg/gs.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace hg {

// Bounded VIF1 transport for the documented VIFcode stream. It implements only
// state-only codes, VU1 memory writes, microprogram uploads, and PATH2
// DIRECT/DIRECTHL. Microprogram activation remains an explicit boundary rather
// than being ignored.
struct Vif1Path {
    std::vector<std::uint8_t> pending;
    // Byte phase of pending[0] within the original VIF DMA stream. Retaining a
    // DIRECT code across DMA calls must retain its 128-bit alignment too.
    std::size_t pending_phase=0;
    std::array<std::uint32_t,4> row{}, column{};
    // VU1 data memory is 16 KiB / 1024 128-bit vectors. UNPACK writes here;
    // it never implies VU microprogram execution.
    std::array<std::uint32_t,4096> vu_mem{};
    // VU1 MicroMem is 16 KiB / 2048 64-bit instruction pairs. MPG only loads
    // the checked storage; MSCAL-family execution is deliberately unsupported.
    // Kept heap-backed so VIF endpoints remain safe to instantiate on the
    // host stack while retaining their fixed, checked hardware capacity.
    std::vector<std::uint64_t> vu_micro_mem=std::vector<std::uint64_t>(2048);
    std::uint32_t cycle=0,offset=0,base=0,itop=0,tops=0,mode=0,mark=0,mask=0;
    bool path3_masked=false;
    bool mark_detected=false;
    static constexpr std::size_t max_pending=1024*1024+16;
    std::uint32_t error_mask=0;
    static bool register_contains(std::uint32_t address) {return address>=0x10003c00u && address<0x10003e00u;}
    void reset_interface() {
        pending.clear();pending_phase=0;row={};column={};
        cycle=offset=base=itop=tops=mode=mark=mask=error_mask=0;
        path3_masked=false;
        mark_detected=false;
        // VU memories are separate storage, not part of the VIF FIFO reset.
    }
    void write_register(std::uint32_t address,std::uint32_t value) {
        if(address==0x10003c30u) {mark=value&0xffffu;mark_detected=false;return;}
        if(address==0x10003c10u) {
            if(value!=1)throw std::runtime_error("unsupported VIF1 FBRST stall/control operation");
            reset_interface();return;
        }
        if(address==0x10003c20u) {
            if(value&~2u)throw std::runtime_error("unsupported VIF1 interrupt/command error masking");
            error_mask=value;return;
        }
        throw std::runtime_error("unsupported VIF1 register write");
    }
    std::uint32_t read_register(std::uint32_t address) const {
        if(address==0x10003c00u) {
            // Supported commands finish synchronously. Retained packet bytes
            // are parser storage, not a modeled 16-qword hardware FIFO; do not
            // fabricate FQC or report idle for an incomplete command.
            if(!pending.empty())throw std::runtime_error("VIF1 STAT during incomplete packet requires FIFO/pipeline modeling");
            return mark_detected?0x40u:0u;
        }
        if(address==0x10003c20u)return error_mask;
        if(address==0x10003c30u)return mark;
        throw std::runtime_error("unsupported VIF1 register read");
    }


    static std::uint32_t word(const std::vector<std::uint8_t>& bytes,std::size_t at) {
        return std::uint32_t(bytes.at(at))|(std::uint32_t(bytes.at(at+1))<<8)|
               (std::uint32_t(bytes.at(at+2))<<16)|(std::uint32_t(bytes.at(at+3))<<24);
    }
    void consume_state(std::uint8_t command,std::uint16_t immediate) {
        switch(command) {
        case 0x00:return; // NOP
        case 0x01:cycle=immediate;return; // STCYCL
        case 0x02:offset=immediate&0x3ff;tops=base;return; // OFFSET initializes VIF1_TOPS from BASE.
        case 0x03:base=immediate&0x3ff;return; // BASE
        case 0x04:itop=immediate&0x3ff;return; // ITOP
        case 0x05:mode=immediate&3;if(mode==3)throw std::runtime_error("reserved VIF1 STMOD mode");return; // STMOD
        case 0x06:path3_masked=(immediate&0x8000)!=0;return; // MSKPATH3
        case 0x07:mark=immediate;mark_detected=true;return; // MARK
        case 0x10:case 0x11:case 0x13:return; // FLUSHE/FLUSH/FLUSHA: synchronous host boundary
        case 0x14:throw std::runtime_error("VIF1 MSCAL requires unavailable AOT VU1 execution");
        case 0x15:throw std::runtime_error("VIF1 MSCALF requires unavailable AOT VU1 execution");
        case 0x17:throw std::runtime_error("VIF1 MSCNT requires unavailable AOT VU1 execution");
        default:throw std::runtime_error("unsupported VIF1 state VIFcode");
        }
    }
    void submit_qword(std::uint64_t low,std::uint64_t high,GifPath& gif,GsRegisterState& gs) {
        for(unsigned i=0;i!=8;++i)pending.push_back(std::uint8_t(low>>(i*8)));
        for(unsigned i=0;i!=8;++i)pending.push_back(std::uint8_t(high>>(i*8)));
        process_pending(gif,gs);
    }
    void submit_dma_tag(std::uint64_t high,GifPath& gif,GsRegisterState& gs) {
        if(!pending.empty())throw std::runtime_error("VIF1 tag inside incomplete VIF packet requires split-tag phase tracking");
        // EE manual p86: the low 64 bits are DMAtag, only the upper
        // two words enter the VIFcode/data stream, at physical byte phase 8.
        pending_phase=8;
        for(unsigned i=0;i!=8;++i)pending.push_back(std::uint8_t(high>>(i*8)));
        process_pending(gif,gs);
    }
    void process_pending(GifPath& gif,GsRegisterState& gs) {
        if(pending.size()>max_pending)throw std::runtime_error("VIF1 packet exceeds bounded path capacity");
        std::size_t cursor=0;
        while(pending.size()-cursor>=4) {
            const auto code=word(pending,cursor);
            if(code&0x80000000u)throw std::runtime_error("VIF1 interrupt-control VIFcode requires interrupt delivery");
            const auto command=std::uint8_t(code>>24)&0x7f;
            const auto immediate=std::uint16_t(code);
            if(command==0x20) { // STMASK: one following word.
                if(pending.size()-cursor<8)break;
                mask=word(pending,cursor+4);cursor+=8;continue;
            }
            if(command==0x30||command==0x31) { // STROW / STCOL, four following words.
                if(pending.size()-cursor<20)break;
                auto& destination=command==0x30?row:column;
                for(unsigned n=0;n!=4;++n)destination[n]=word(pending,cursor+4+n*4);
                cursor+=20;continue;
            }
            if(command==0x4a) { // MPG: NUM following 64-bit microinstructions.
                const std::size_t instructions=(code>>16)&0xff ? (code>>16)&0xff : 256;
                const std::size_t destination=immediate;
                if(destination>vu_micro_mem.size() || instructions>vu_micro_mem.size()-destination)
                    throw std::runtime_error("VIF1 MPG exceeds VU1 microprogram memory");
                if((pending_phase+cursor+4)%8)
                    throw std::runtime_error("VIF1 MPG payload is not 64-bit aligned");
                const std::size_t bytes=instructions*8;
                if(pending.size()-cursor<4+bytes)break;
                for(std::size_t n=0;n<instructions;++n) {
                    const auto data=cursor+4+n*8;
                    vu_micro_mem[destination+n]=std::uint64_t(word(pending,data))|
                                                 (std::uint64_t(word(pending,data+4))<<32);
                }
                cursor+=4+bytes;continue;
            }
            if(command>=0x60) { // UNPACK: documented scalar/vector forms.
                const auto format=command&0xf;
                const auto masked=(command&0x10)!=0;
                const bool rgba5=format==0xf;
                const unsigned bits=(format==0||format==0xc)?32:
                                    (format==1||format==0xd)?16:
                                    (format==2||format==0xe)?8:
                                    (format==4||format==8)?32:
                                    (format==5||format==9)?16:
                                    (format==6||format==10)?8:rgba5?16:0;
                if(!bits)throw std::runtime_error("unsupported VIF1 UNPACK format");
                const bool scalar=format<=2;
                const unsigned input_fields=scalar?1:(format>=4&&format<=6)?2:(format>=8&&format<=10)?3:4;
                const std::size_t vectors=(code>>16)&0xff ? (code>>16)&0xff : 256;
                const std::size_t destination=(immediate&0x8000?tops:0)+(immediate&0x3ff);
                auto wl=std::size_t(cycle&0xff),cl=std::size_t(cycle>>8);
                if(!wl)wl=256;if(!cl)cl=256;
                if(cl<wl&&!masked)throw std::runtime_error("VIF1 UNPACK fill-cycle requires an explicit mask");
                const auto input_vectors=cl>=wl?vectors:cl*(vectors/wl)+std::min(vectors%wl,cl);
                const auto unit_bytes=std::size_t(rgba5?2:input_fields*bits/8);
                const std::size_t bytes=(input_vectors*unit_bytes+3)&~std::size_t(3);
                if(pending.size()-cursor<4+bytes)break;
                std::size_t data=cursor+4;
                const auto extend=[&](std::uint32_t value) {
                    if(bits==32)return value;
                    if(immediate&(1u<<14))return value;
                    const auto sign=std::uint32_t(1)<<(bits-1);
                    return (value^sign)-sign;
                };
                for(std::size_t n=0;n<vectors;++n) {
                    const auto in_cycle=n%wl;
                    const auto has_input=cl>=wl||in_cycle<cl;
                    const auto vector=cl>=wl?destination+cl*(n/wl)+in_cycle:destination+n;
                    if(vector>=1024)throw std::runtime_error("VIF1 UNPACK exceeds VU1 data memory");
                    // V2/V3 leave trailing lanes indeterminate. Validate the
                    // whole vector before changing memory so an unsupported
                    // packet cannot partially commit X/Y/Z before faulting W.
                    for(unsigned field=0;field!=4;++field) {
                        const auto write_cycle=std::min<std::size_t>(in_cycle,3);
                        const auto selector=masked?unsigned((mask>>(2*(write_cycle*4+field)))&3):0;
                        if(selector==0&&(!has_input||(!scalar&&field>=input_fields)))
                            throw std::runtime_error("VIF1 UNPACK indeterminate component requires an explicit mask");
                    }
                    std::uint16_t packed=0;
                    if(has_input&&rgba5)packed=std::uint16_t(pending.at(data))|(std::uint16_t(pending.at(data+1))<<8);
                    for(unsigned field=0;field!=4;++field) {
                        const auto write_cycle=std::min<std::size_t>(in_cycle,3);
                        const auto selector=masked?unsigned((mask>>(2*(write_cycle*4+field)))&3):0;
                        if(selector==3)continue; // Documented masked write: retain VU memory.
                        if(selector==1) {vu_mem[vector*4+field]=row[field];continue;}
                        if(selector==2) {vu_mem[vector*4+field]=column[write_cycle];continue;}
                        if(rgba5) {
                            const auto channel=field==0?(packed&31):field==1?((packed>>5)&31):field==2?((packed>>10)&31):((packed>>15)&1);
                            vu_mem[vector*4+field]=channel<<(field==3?7:3);continue;
                        }
                        const auto byte_offset=data+(scalar?0:field*(bits/8));
                        std::uint32_t value=0;
                        for(unsigned byte=0;byte<bits/8;++byte)value|=std::uint32_t(pending.at(byte_offset+byte))<<(byte*8);
                        value=extend(value);
                        if(mode) {
                            value+=row[field];
                            if(mode==2)row[field]=value;
                        }
                        vu_mem[vector*4+field]=value;
                    }
                    if(has_input)data+=unit_bytes;
                }
                cursor+=4+bytes;continue;
            }
            if(command==0x50||command==0x51) { // DIRECT / DIRECTHL
                const std::size_t units=immediate?immediate:65536,bytes=units*16;
                if((pending_phase+cursor+4)%16)throw std::runtime_error("VIF1 DIRECT payload is not GIF-qword aligned");
                if(pending.size()-cursor<4+bytes)break;
                std::size_t data=cursor+4;
                for(std::size_t unit=0;unit!=units;++unit,data+=16) {
                    const auto qlow=std::uint64_t(word(pending,data))|(std::uint64_t(word(pending,data+4))<<32);
                    const auto qhigh=std::uint64_t(word(pending,data+8))|(std::uint64_t(word(pending,data+12))<<32);
                    gif.submit_qword(qlow,qhigh,gs);
                }
                cursor+=4+bytes;continue;
            }
            consume_state(command,immediate);cursor+=4;
        }
        if(cursor) {
            pending.erase(pending.begin(),pending.begin()+cursor);
            pending_phase=(pending_phase+cursor)%16;
        }
    }
};

} // namespace hg
