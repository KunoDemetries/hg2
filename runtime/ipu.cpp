#include "hg/ipu.hpp"
#include "hg/ipu_dct_tables.hpp"
#include "hg/ipu_dct_lookup.hpp"
#include <algorithm>
#include <cmath>

namespace hg {
namespace {
// H.262 figures 7-2/7-3: row/column position -> transmitted scan index.
constexpr unsigned scan[2][64]={
 {0,1,5,6,14,15,27,28,2,4,7,13,16,26,29,42,3,8,12,17,25,30,41,43,9,11,18,24,31,40,44,53,10,19,23,32,39,45,52,54,20,22,33,38,46,51,55,60,21,34,37,47,50,56,59,61,35,36,48,49,57,58,62,63},
 {0,4,6,20,22,36,38,52,1,5,7,21,23,37,39,53,2,8,19,24,34,40,50,54,3,9,18,25,35,41,51,55,10,17,26,30,42,46,56,60,11,16,27,31,43,47,57,61,12,15,28,32,44,48,58,62,13,14,29,33,45,49,59,63}
};
constexpr unsigned nonlinear[32]={0,1,2,3,4,5,6,7,8,10,12,14,16,18,20,22,24,28,32,36,40,44,48,52,56,64,72,80,88,96,104,112};
constexpr unsigned dc_bits[2][12]={{3,2,2,3,3,4,5,6,7,8,9,9},{2,2,2,3,4,5,6,7,8,9,10,10}};
constexpr unsigned dc_code[2][12]={{4,0,1,5,6,14,30,62,126,254,510,511},{0,1,2,6,14,30,62,126,254,510,1022,1023}};
}
void Ipu::begin_block() {
    if(pending&0x04000000u)dc_predictors.fill(128<<((settings>>16)&3));
    block=0;coefficient=0;block_phase=pending&0x08000000u?0:2;output_cursor=0;
    coefficients.fill(0);pixels.fill(0);
}
std::uint32_t Ipu::rgba(unsigned y,unsigned cb,unsigned cr,std::uint32_t thresholds) {
    // EE manual pp205-206 coefficients and half-bit quantization. Measured
    // profile clamps Y-16 and quantizes signed green contributions separately;
    // this differs from literal subtraction in the manual (docs/ORACLE.md).
    const auto floor_div=[](int v,int d){return v>=0?v/d:-((-v+d-1)/d);};
    const int luma=149*std::max(0,int(y)-16)/64,c=int(cb)-128,r=int(cr)-128;
    const auto channel=[&](int v){return unsigned(std::clamp(floor_div(v+1,2),0,255));};
    unsigned red=channel(luma+floor_div(204*r,64));
    unsigned green=channel(luma+floor_div(-50*c,64)+floor_div(-104*r,64));
    unsigned blue=channel(luma+floor_div(258*c,64));
    const unsigned maximum=std::max({red,green,blue});
    if(maximum<(thresholds&511))return 0;
    const unsigned alpha=maximum<((thresholds>>16)&511)?64:128;
    return red|(green<<8)|(blue<<16)|(alpha<<24);
}
void Ipu::service_csc() {
    while(csc_remaining) {
        while(csc_bytes<raw_pixels.size()) {
            if(!ensure(8))return;
            // An aligned buffered lane already contains eight consecutive
            // stream bytes. Consume it directly instead of running the bit
            // extractor eight times. Keep partial/unaligned input on the
            // ordinary path and preserve the exact FIFO consumption boundary.
            if(!(bit_position&63) && raw_pixels.size()-csc_bytes>=8) {
                const auto& q=buffered[bit_position/128];
                const auto lane=(bit_position&64)?q.high:q.low;
                for(unsigned n=0;n<8;++n)raw_pixels[csc_bytes+n]=std::uint8_t(lane>>(8*n));
                csc_bytes+=8;advance(64);
            } else {raw_pixels[csc_bytes++]=std::uint8_t(peek(8));advance(8);}
        }
        while(csc_pixel<256) {
            if(output_count==output.size())return;
            IpuQuad q;
            for(unsigned lane=0;lane<4;++lane) {
                const unsigned n=csc_pixel++,chroma=(n/32)*8+(n%16)/2;
                const auto color=rgba(raw_pixels[n],raw_pixels[256+chroma],raw_pixels[320+chroma],thresholds);
                (lane<2?q.low:q.high)|=std::uint64_t(color)<<((lane%2)*32);
            }
            push_output(q);
        }
        --csc_remaining;csc_bytes=csc_pixel=0;
    }
    busy=false;
}
void Ipu::transform_block() {
    std::array<int,64> reconstructed{};
    const unsigned precision=(settings>>16)&3,mode=(settings>>20)&1;
    const bool intra=pending&0x08000000u;
    const unsigned qcode=(pending>>16)&31,scale=settings&0x00400000u?nonlinear[qcode]:qcode*2;
    for(unsigned n=0;n<64;++n) {
        const auto index=scan[mode][n];
        const unsigned matrix_index=scan[0][n];
        const auto& q=(intra?intra_iq:non_intra_iq)[matrix_index/16];
        const auto lane=matrix_index%16<8?q.low:q.high;
        const unsigned weight=(lane>>((matrix_index%8)*8))&255;
        const int level=coefficients[index];
        const int product=(2*level+(level>0?1:level<0?-1:0))*int(weight*scale);
        const int value=intra?(n==0?coefficients[0]*(8>>precision):level*int(weight*scale)/16):
            (product>=0?product/32:-((-product+31)/32));
        reconstructed[n]=std::clamp(value,-2048,2047);
    }
    // The measured intra profile omits mismatch correction (docs/ORACLE.md).
    // Do not infer non-intra behavior or physical-console parity from this.
    // Direct separable mathematical IDCT, H.262 Annex A. No imported fast-IDCT
    // algorithm. Host floating-point and PS2 integer-rounding parity are unverified.
    static const auto basis=[] {
        std::array<std::array<double,8>,8> values{};
        const double pi=std::acos(-1.0);
        for(unsigned x=0;x<8;++x)for(unsigned u=0;u<8;++u)
            values[x][u]=(u?1.0:std::sqrt(0.5))*std::cos((2*x+1)*u*pi/16.0)/2.0;
        return values;
    }();
    // A block with no reconstructed AC terms is spatially constant. Retain
    // the same two basis multiplications and final rounding as the full sum.
    const bool dc_only=std::all_of(reconstructed.begin()+1,reconstructed.end(),[](int v){return v==0;});
    const double dc_value=reconstructed[0]*basis[0][0]*basis[0][0];
    double horizontal[8][8]{};
    // Process neighboring outputs together so the host compiler can vectorize
    // across X. Each output still adds U, then V, in the original order.
    if(!dc_only)for(unsigned v=0;v<8;++v)for(unsigned u=0;u<8;++u)
        for(unsigned x=0;x<8;++x)horizontal[v][x]+=reconstructed[v*8+u]*basis[x][u];
    for(unsigned y=0;y<8;++y) {
        double values[8]{};
        if(dc_only)for(auto& value:values)value=dc_value;
        else for(unsigned v=0;v<8;++v)for(unsigned x=0;x<8;++x)
            values[x]+=horizontal[v][x]*basis[y][v];
        for(unsigned x=0;x<8;++x) {
            const unsigned destination=block<4?((block/2)*8+y)*16+(block%2)*8+x:256+(block-4)*64+y*8+x;
            pixels[destination]=std::int16_t(std::clamp(int(std::floor(values[x]+0.5+1e-9)),intra?0:-256,255));
        }
    }
}
void Ipu::service_block() {
    const bool intra=pending&0x08000000u;
    if(block_phase==2) {
        if(!ensure(32))return;
        const auto prefix=peek(32);
        const IpuCbp* found=nullptr;
        for(const auto& entry:ipu_cbp)if((prefix>>(32-entry.bits))==entry.code){found=&entry;break;}
        if(!found || !found->mask)throw std::runtime_error("IPU invalid 4:2:0 coded block pattern");
        status=found->mask<<8;advance(found->bits);block_phase=0;
    }
    while(block<6) {
        if(!intra && !(status&(1u<<(13-block)))){++block;continue;}
        if(block_phase==0) {
            if(!ensure(32))return;
            if(!intra) {coefficients.fill(0);coefficient=0;block_phase=1;}
            else {
            const unsigned chroma=block>=4;
            const auto prefix=peek(32);
            unsigned size=0;
            for(;size<12;++size)if((prefix>>(32-dc_bits[chroma][size]))==dc_code[chroma][size])break;
            if(size==12)throw std::runtime_error("IPU invalid intra DC prefix");
            const auto bits=dc_bits[chroma][size];
            // Longest DC prefix plus differential fits the available 32 bits.
            advance(bits);int delta=0;
            if(size){const auto raw=peek(size);delta=raw&(1u<<(size-1))?int(raw):int(raw)+1-int(1u<<size);advance(size);}
            const unsigned component=block<4?0:block-3;
            const int prediction=dc_predictors[component]+delta;
            if(prediction<0 || prediction>=int(1u<<(8+((settings>>16)&3))))
                throw std::runtime_error("IPU intra DC predictor outside MPEG2 range");
            dc_predictors[component]=prediction;coefficients.fill(0);coefficients[0]=prediction;
            coefficient=1;block_phase=1;
            }
        }
        if(!ensure(32))return;
        const bool table_one=intra && (settings&0x00200000u);
        if(!intra && coefficient==0 && peek(1)==1) {
            coefficients[0]=peek(2)&1?-1:1;advance(2);coefficient=1;continue;
        }
        const unsigned eob_bits=table_one?4:2,eob_code=table_one?6:2;
        if(peek(eob_bits)==eob_code) {
            advance(eob_bits);transform_block();++block;block_phase=0;continue;
        }
        unsigned run=0,bits=0;int level=0;
        if(peek(6)==1) {
            // MPEG2 escape: six-bit run and signed twelve-bit level.
            const auto raw=peek(24);run=(raw>>12)&63;level=int(raw&4095);
            if(level&2048)level-=4096;
            if(level==0 || level==-2048)throw std::runtime_error("IPU forbidden MPEG2 escape level");
            bits=24;
        } else {
            // ensure(32) above makes one immutable prefix sufficient for every
            // candidate, including its sign. No stream advancement during lookup.
            const auto prefix=peek(32);
            const auto* entry=lookup_ipu_dct(table_one,prefix);
            if(!entry)throw std::runtime_error("IPU invalid DCT coefficient prefix");
            run=entry->run;level=entry->level;bits=entry->bits+1;
            if((prefix>>(32-bits))&1)level=-level;
        }
        if(coefficient+run>=64)throw std::runtime_error("IPU DCT coefficient run exceeds block");
        coefficients[coefficient+run]=level;coefficient+=run+1;advance(bits);
    }
    while(output_cursor<48 && output_count<output.size()) {
        IpuQuad q;
        for(unsigned n=0;n<8;++n) {
            const auto sample=std::uint16_t(pixels[output_cursor*8+n]);
            (n<4?q.low:q.high)|=std::uint64_t(sample)<<((n%4)*16);
        }
        push_output(q);++output_cursor;
    }
    if(output_cursor<48)return;
    if(!ensure(32))return;
    // The first eight zero bits indicate start-code alignment per EE manual p192.
    if(peek(8)==0) {
        const unsigned padding=(8-bit_position%8)%8;
        if(padding){if(peek(padding)!=0)throw std::runtime_error("IPU nonzero start-code alignment");advance(padding);}
        if(!ensure(32))return;
        if((peek(32)>>8)!=1)throw std::runtime_error("IPU invalid start code following block");
        status|=0x8000;
    }
    top=peek(32);busy=false;
}
}
