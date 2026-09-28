#pragma once
#include <array>
#include <cstdint>
#include <stdexcept>
#include "hg/ipu_vlc.hpp"

namespace hg {
struct IpuQuad { std::uint64_t low=0,high=0; };

// Independent startup subset from EE User's Manual chapter 8. It retains the
// shared bit stream, VLC, intra block decoding and bounded color conversion.
struct Ipu {
    std::array<IpuQuad,8> input{},output{};
    unsigned output_count=0;
    unsigned input_count=0;
    std::array<IpuQuad,4> intra_iq{},non_intra_iq{};
    std::array<IpuQuad,2> vq{};
    std::uint32_t settings=0,bit_position=0,thresholds=0;
    bool busy=false;

    std::array<int,3> dc_predictors{};
    std::array<int,64> coefficients{};
    std::array<std::int16_t,384> pixels{};
    unsigned block=0,coefficient=0,block_phase=0,output_cursor=0;
    void begin_block();
    void service_block();
    void transform_block();
    std::array<std::uint8_t,384> raw_pixels{};
    unsigned csc_bytes=0,csc_remaining=0,csc_pixel=0;
    static std::uint32_t rgba(unsigned y,unsigned cb,unsigned cr,std::uint32_t thresholds);
    void service_csc();
    static bool register_contains(std::uint32_t address) {
        return address==0x10002000 || address==0x10002010 ||
               address==0x10002020 || address==0x10002030;
    }
    static bool input_fifo_contains(std::uint32_t address) { return address==0x10007010; }
    std::uint32_t control() const {
        return settings | status | (output_count<<4) | (busy?0x80000000u:0) | (input_count&15u);
    }
    std::array<IpuQuad,2> buffered{};
    unsigned buffered_count=0,table_bytes=0,forward=0;
    std::uint32_t pending=0,result=0,top=0,status=0;
    bool stream_enabled=false,vlc_decoded=false;
    std::uint32_t position() const {return (buffered_count<<16)|(input_count<<8)|bit_position;}
    void reset() {
        busy=false;output_count=0;input_count=buffered_count=bit_position=table_bytes=forward=0;
        pending=0;block_phase=block=output_cursor=0;status=0;stream_enabled=false;vlc_decoded=false;
        csc_bytes=csc_remaining=csc_pixel=0;
    }
    bool ensure(unsigned bits) {
        while(buffered_count*128<bit_position+bits && input_count && buffered_count<2) {
            buffered[buffered_count++]=input[0];
            for(unsigned n=1;n<input_count;++n)input[n-1]=input[n];
            --input_count;
        }
        return buffered_count*128>=bit_position+bits;
    }
    std::uint32_t peek(unsigned bits) const {
        std::uint32_t value=0;
        unsigned offset=bit_position;
        while(bits) {
            const auto& q=buffered[offset/128];
            const auto byte=(offset%128)/8;
            const auto lane=byte<8?q.low:q.high;
            const unsigned available=8-offset%8;
            const unsigned take=bits<available?bits:available;
            const auto part=(lane>>((byte%8)*8+available-take))&((1u<<take)-1);
            value=(value<<take)|std::uint32_t(part);
            offset+=take;bits-=take;
        }
        return value;
    }
    void advance(unsigned bits) {
        bit_position+=bits;
        while(bit_position>=128 && buffered_count) {
            bit_position-=128;buffered[0]=buffered[1];--buffered_count;
        }
    }
    template<std::size_t N> std::uint32_t lookup(const IpuVlc (&entries)[N]) const {
        for(const auto& entry:entries) {
            if(entry.mpeg1!=-1 && entry.mpeg1!=int((settings>>23)&1))continue;
            if(peek(entry.length)==entry.bits)return entry.result;
        }
        throw std::runtime_error("IPU VDEC prefix absent from hardware specification table");
    }
    std::uint32_t decode_vlc() const {
        switch((pending>>26)&3) {
        case 0:return lookup(ipu_address);
        case 1:switch((settings>>24)&7) {
            case 1:return lookup(ipu_type_i);
            case 2:return lookup(ipu_type_p);
            case 3:return lookup(ipu_type_b);
            case 4:return lookup(ipu_type_d);
            default:throw std::runtime_error("IPU VDEC reserved picture type");
            }
        case 2:return lookup(ipu_motion);
        case 3:return lookup(ipu_dmvector);
        }
        throw std::runtime_error("IPU VDEC invalid table");
    }
    void service() {
        if(!stream_enabled)return;
        if(!busy){ensure(1);return;}
        if(forward) {
            if(!ensure(forward))return;
            advance(forward);forward=0;
        }
        const auto code=pending>>28;
        if(code==2){service_block();return;}
        if(code==7){service_csc();return;}
        if(code==3) {
            if(!vlc_decoded) {
                if(!ensure(32))return;
                result=decode_vlc();
                if(result==0)status|=0x4000;
                advance(result>>16);vlc_decoded=true;
            }
            if(!ensure(32))return;
            top=peek(32);busy=false;return;
        }
        if(code==4) {
            if(!ensure(32))return;
            result=top=peek(32);busy=false;return;
        }
        const unsigned count=code==5?64:32;
        while(table_bytes<count) {
            if(!ensure(8))return;
            auto& q=code==6?vq[table_bytes/16]:
                (pending&0x08000000u?non_intra_iq[table_bytes/16]:intra_iq[table_bytes/16]);
            auto& lane=table_bytes%16<8?q.low:q.high;
            const auto shift=(table_bytes%8)*8;
            lane=(lane&~(0xffull<<shift))|(std::uint64_t(peek(8))<<shift);
            advance(8);++table_bytes;
        }
        busy=false;ensure(1);
    }
    bool push_output(IpuQuad value) {
        if(output_count==output.size())return false;
        output[output_count++]=value;return true;
    }
    void pop_output() {
        if(!output_count)throw std::runtime_error("IPU output FIFO underflow");
        for(unsigned n=1;n<output_count;++n)output[n-1]=output[n];
        --output_count;service();
    }
    void push(IpuQuad value) {
        if(input_count==input.size())throw std::runtime_error("IPU input FIFO overflow");
        input[input_count++]=value;service();
    }
    void command(std::uint32_t value) {
        const auto code=value>>28;
        if(code==0) {
            if(value&0x0fffff80u)throw std::runtime_error("IPU BCLR reserved option bits are set");
            const auto retained_output=output_count;reset();output_count=retained_output;
            bit_position=value&0x7fu;stream_enabled=true;return;
        }
        if(busy)throw std::runtime_error("IPU command issued while busy");
        if(code==7) {
            if(value&0x0ffff800u)throw std::runtime_error("IPU CSC currently requires RGB32 without dithering");
            if(!(value&0x7ffu) || bit_position || output_count)
                throw std::runtime_error("IPU CSC requires nonzero count and aligned drained stream");
            pending=value;csc_remaining=value&0x7ffu;csc_bytes=csc_pixel=0;
            busy=true;status=0;stream_enabled=true;service();return;
        }
        if(code==2) {
            if((value&0x01e0ffc0u) || (value&63)>32)
                throw std::runtime_error("IPU BDEC reserved/forward bits invalid");
            if((value&0x02000000u) || (settings&0x00800000u))
                throw std::runtime_error("IPU BDEC currently requires MPEG2 frame blocks");
            if(((settings>>16)&3)==3 || !((value>>16)&31))
                throw std::runtime_error("IPU BDEC reserved precision/quantizer");
            if(output_count)throw std::runtime_error("IPU BDEC with undrained previous output");
            pending=value;forward=value&63;status=0x3f00;busy=true;stream_enabled=true;
            begin_block();service();return;
        }
        if(code==3 || code==4 || code==5 || code==6) {
            const auto mask=code==3?0x03ffffc0u:code==4?0x0fffffc0u:code==5?0x07ffffc0u:0x0fffffffu;
            if((value&mask) || (code!=6 && (value&63)>32))
                throw std::runtime_error("IPU command reserved/forward bits invalid");
            const auto fb=code==6?0:value&63;
            if(bit_position+fb>=128 && !buffered_count && !input_count)
                throw std::runtime_error("IPU empty forward crossing remains unverified");
            if(code==3 && ((value>>26)&3)==1 && (((settings>>24)&7)<1 || ((settings>>24)&7)>4))
                throw std::runtime_error("IPU VDEC reserved picture type");
            pending=value;forward=fb;table_bytes=0;status=0;vlc_decoded=false;busy=true;stream_enabled=true;service();return;
        }
        if(code==9) {
            if(value&0x0e00fe00u)throw std::runtime_error("IPU SETTH reserved option bits are set");
            thresholds=value&0x01ff01ffu;status=0;return;
        }
        throw std::runtime_error("IPU decode command is not implemented");
    }
    std::uint64_t read(std::uint32_t address,unsigned size) const {
        if(address==0x10002010 && size==4)return control();
        if(address==0x10002020 && size==4)return position();
        if((address==0x10002000 || address==0x10002030) && (size==4 || size==8)) {
            const bool unavailable=busy && (address==0x10002030 || (pending>>28==3 || pending>>28==4));
            return (address==0x10002000?result:top)|
                (size==8 && unavailable?1ull<<63:0);
        }
        throw std::runtime_error("IPU register read or width is not implemented");
    }
    void write(std::uint32_t address,unsigned size,std::uint64_t value) {
        if(address==0x10002010 && size==4) {
            settings=std::uint32_t(value)&0x07f30000u;
            if(value&0x40000000u)reset();
            return;
        }
        if(address==0x10002000 && (size==4 || size==8)) {command(std::uint32_t(value));return;}
        throw std::runtime_error("IPU register write or width is not implemented");
    }
};
}
