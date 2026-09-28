#pragma once

#include "hg/fpu.hpp"
#include "hg/gs.hpp"
#include <algorithm>
#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#if defined(_MSC_VER)
#define HG_VU_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define HG_VU_INLINE inline __attribute__((always_inline))
#else
#define HG_VU_INLINE inline
#endif

namespace hg {

struct Vif1Path;
struct Vu1State;
// Host arithmetic specialization; false leaves all architectural state untouched.
bool try_vu_full_multiply_acc(Vu1State&,unsigned source,unsigned other,unsigned broadcast,bool add);
bool try_vu_full_madd_vector(Vu1State&,unsigned destination,unsigned source,unsigned other,unsigned broadcast);
using Vu1AotExecutor=void(*)(Vif1Path&,std::uint16_t,GifPath&,GsRegisterState&);

// Host-only overlap bookkeeping. Track only first-touched VU memory locations so
// the native overlap path can merge exact concurrent VIF/VU writes without a
// whole 16 KiB baseline copy or a 4096-word scan on every microprogram start.
struct Vu1MemoryWriteTracker {
    std::bitset<4096> dirty_words{};
    std::bitset<1024> dirty_vectors{};
    std::array<std::uint32_t,4096> baseline_words{};
    std::array<std::uint8_t,1024> baseline_defined{};
    std::vector<std::uint16_t> word_indices;
    std::vector<std::uint16_t> vector_indices;
    Vu1MemoryWriteTracker() {word_indices.reserve(256);vector_indices.reserve(128);}
    void clear() {
        for(const auto index:word_indices)dirty_words.reset(index);
        for(const auto index:vector_indices)dirty_vectors.reset(index);
        word_indices.clear();vector_indices.clear();
    }
    void before_write(const std::array<std::uint32_t,4096>& memory,
                      const std::array<std::uint8_t,1024>& defined,
                      std::size_t word_index,std::size_t vector_index) {
        if(word_index>=memory.size()||vector_index>=defined.size())
            throw std::runtime_error("VU1 host write tracker index exceeds data memory");
        if(!dirty_words.test(word_index)) {
            dirty_words.set(word_index);baseline_words[word_index]=memory[word_index];
            word_indices.push_back(std::uint16_t(word_index));
        }
        if(!dirty_vectors.test(vector_index)) {
            dirty_vectors.set(vector_index);baseline_defined[vector_index]=defined[vector_index];
            vector_indices.push_back(std::uint16_t(vector_index));
        }
    }
};

struct Vu1State {
    std::array<std::array<std::uint32_t,4>,32> vf{};
    // One bit per lane (bit0=X ... bit3=W). Existing register contents remain
    // conservatively defined; VIF can explicitly introduce documented
    // indeterminate lanes which LQ propagates without inventing a value.
    std::array<std::uint8_t,32> vf_defined{};
    std::array<std::uint16_t,16> vi{};
    std::array<std::uint32_t,4> acc{};
    std::uint8_t acc_defined=0;
    std::uint32_t clip=0,status=0,mac=0,q=0,i=0,p=0;
    std::uint32_t pending_q=0;
    std::uint32_t pending_p=0;
    unsigned q_cycles_remaining=0;
    unsigned p_cycles_remaining=0;
    bool q_pending=false;
    bool p_pending=false;
    std::uint16_t tpc=0;
    // Sony VU manual 3.4.1/3.4.4: per-field FMAC data hazards stall both
    // pipelines. Compiled call sites supply register IDs/masks; no decoding.
    std::uint64_t issue_cycle=0;
    std::array<std::array<std::uint64_t,4>,32> vf_ready{};
    std::array<std::uint64_t,16> vi_ready{};

    HG_VU_INLINE void advance_pipeline(std::uint64_t cycles) {
        issue_cycle+=cycles;
        if(q_pending) {
            if(cycles>=q_cycles_remaining){q=pending_q;q_pending=false;q_cycles_remaining=0;}
            else q_cycles_remaining-=unsigned(cycles);
        }
        if(p_pending) {
            if(cycles>=p_cycles_remaining){p=pending_p;p_pending=false;p_cycles_remaining=0;}
            else p_cycles_remaining-=unsigned(cycles);
        }
    }
    HG_VU_INLINE void require_vf(unsigned reg,unsigned mask) {
        if(reg>=vf_ready.size())throw std::runtime_error("VU1 hazard VF index exceeds register file");
        if(!reg)return;
        auto ready=issue_cycle;
        for(unsigned lane=0;lane<4;++lane)if(mask&(8u>>lane))ready=std::max(ready,vf_ready[reg][lane]);
        advance_pipeline(ready-issue_cycle);
    }
    HG_VU_INLINE void require_vi(unsigned reg) {
        if(reg>=vi_ready.size())throw std::runtime_error("VU1 hazard VI index exceeds register file");
        if(reg&&vi_ready[reg]>issue_cycle)advance_pipeline(vi_ready[reg]-issue_cycle);
    }
    HG_VU_INLINE void produced_vf(unsigned reg,unsigned mask) {
        if(reg>=vf_ready.size())throw std::runtime_error("VU1 hazard VF index exceeds register file");
        if(reg)for(unsigned lane=0;lane<4;++lane)if(mask&(8u>>lane))vf_ready[reg][lane]=issue_cycle+4;
    }
    HG_VU_INLINE void produced_vi(unsigned reg,unsigned latency) {
        if(reg>=vi_ready.size())throw std::runtime_error("VU1 hazard VI index exceeds register file");
        if(reg)vi_ready[reg]=issue_cycle+latency;
    }

    Vu1State() { vf[0][3]=0x3f800000u;vf_defined.fill(0x0fu); }

    std::uint16_t read_vi(unsigned index) const {
        if(index>=vi.size())throw std::runtime_error("VU1 integer register index exceeds VI file");
        return index?vi[index]:0;
    }
    void write_vi(unsigned index,std::uint16_t value) {
        if(index>=vi.size())throw std::runtime_error("VU1 integer register index exceeds VI file");
        if(index)vi[index]=value;
    }
    HG_VU_INLINE std::uint32_t read_vf(unsigned index,unsigned lane) const {
        if(index>=vf.size()||lane>=4)throw std::runtime_error("VU1 vector register index exceeds VF file");
        if(!index)return lane==3?0x3f800000u:0u;
        if(!(vf_defined[index]&(1u<<lane)))throw std::runtime_error("VU1 vector operation reads an undefined VF lane");
        return vf[index][lane];
    }
    HG_VU_INLINE void write_vf(unsigned index,unsigned lane,std::uint32_t value) {
        if(index>=vf.size()||lane>=4)throw std::runtime_error("VU1 vector register index exceeds VF file");
        if(!index)return;
        vf[index][lane]=value;
        vf_defined[index]|=std::uint8_t(1u<<lane);
    }
    void move_vector(unsigned destination,unsigned source,unsigned dest) {
        if(destination>=vf.size()||source>=vf.size())throw std::runtime_error("VU1 MOVE register index exceeds VF file");
        if(!destination)return;
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane)) {
            const auto bit=std::uint8_t(1u<<lane);
            vf[destination][lane]=source?vf[source][lane]:(lane==3?0x3f800000u:0u);
            if(!source || (vf_defined[source]&bit))vf_defined[destination]|=bit;
            else vf_defined[destination]&=std::uint8_t(~bit);
        }
    }
    static bool float_less(std::uint32_t a,std::uint32_t b) {
        if(!((a>>23)&255u))a&=0x80000000u;
        if(!((b>>23)&255u))b&=0x80000000u;
        if((a^b)&0x80000000u)return (a&0x80000000u)!=0;
        if(a&0x80000000u)return (a&0x7fffffffu)>(b&0x7fffffffu);
        return (a&0x7fffffffu)<(b&0x7fffffffu);
    }
    static std::uint32_t integer_to_float(std::uint32_t value,unsigned fractional_bits) {
        const auto signed_value=std::int64_t(std::int32_t(value));
        if(!signed_value)return 0;
        const auto sign=signed_value<0?0x80000000u:0u;
        const auto magnitude=std::uint64_t(signed_value<0?-signed_value:signed_value);
        unsigned top=0;for(auto probe=magnitude;probe>>=1;)++top;
        const int exponent=127+int(top)-int(fractional_bits);
        if(exponent<=0)return sign;
        std::uint64_t mantissa=top<=23?magnitude<<(23-top):magnitude>>(top-23);
        return sign|(std::uint32_t(exponent)<<23)|(std::uint32_t(mantissa)&0x7fffffu);
    }
    static std::uint32_t float_to_integer(std::uint32_t value,unsigned fractional_bits) {
        const auto exponent=(value>>23)&255u;
        if(!exponent)return 0;
        const auto mantissa=std::uint64_t((value&0x7fffffu)|0x800000u);
        const int shift=int(exponent)-127+int(fractional_bits)-23;
        std::uint64_t magnitude;
        if(shift>=0) {
            if(shift>=40)return value&0x80000000u?0x80000000u:0x7fffffffu;
            magnitude=mantissa<<shift;
        } else magnitude=(-shift>=64)?0:mantissa>>(-shift);
        if(value&0x80000000u) {
            if(magnitude>=0x80000000ull)return 0x80000000u;
            return std::uint32_t(-std::int64_t(magnitude));
        }
        return magnitude>0x7fffffffull?0x7fffffffu:std::uint32_t(magnitude);
    }
    void tick_q() {
        if(!q_pending)return;
        if(q_cycles_remaining && --q_cycles_remaining)return;
        q=pending_q;q_pending=false;
    }
    void wait_q() {
        if(q_pending)advance_pipeline(q_cycles_remaining);
    }
    void tick_p() {
        if(!p_pending)return;
        if(p_cycles_remaining && --p_cycles_remaining)return;
        p=pending_p;p_pending=false;
    }
    void wait_p() {
        if(p_pending){p=pending_p;p_cycles_remaining=0;p_pending=false;}
    }
    void erleng(unsigned source) {
        // VU1 EFU is single-issue. Starting another EFU operation stalls until
        // the previous P writeback completes. ERLENG itself does not change flags.
        if(p_pending)wait_p();
        const auto x=read_vf(source,0),y=read_vf(source,1),z=read_vf(source,2);
        const auto xx=Fpu::product(x,x).bits,yy=Fpu::product(y,y).bits,zz=Fpu::product(z,z).bits;
        Fpu sum_unit;const auto xy=sum_unit.add(xx,yy);const auto xyz=sum_unit.add(xy,zz);
        // Sony documents P=1/sqrt(x^2+y^2+z^2). The project uses its existing
        // independently derived bit-arithmetic FPU profile here; exact EFU last-bit
        // parity remains an explicit validation item rather than host FP behavior.
        Fpu root_unit;const auto root=root_unit.sqrt_exact(xyz);
        Fpu divide_unit;pending_p=divide_unit.divide(0x3f800000u,root);
        p_cycles_remaining=24;p_pending=true;
    }
    void move_from_p(unsigned destination,unsigned dest) {
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane))write_vf(destination,lane,p);
    }
    void divide(unsigned fs,unsigned ft,unsigned fsf,unsigned ftf) {
        if(q_pending)throw std::runtime_error("VU1 DIV started before the prior Q result completed");
        Fpu arithmetic;
        const auto numerator=read_vf(fs,fsf),denominator=read_vf(ft,ftf);
        pending_q=arithmetic.divide(numerator,denominator);
        const auto ne=(numerator>>23)&255u,de=(denominator>>23)&255u;
        const unsigned flags=!de?(ne?32u:16u):0u;
        status=(status&~48u)|flags|(flags<<6);
        q_cycles_remaining=7;
        q_pending=true;
    }
    void integer_add(unsigned destination,unsigned source,std::uint16_t value,bool subtract=false) {
        const auto result=std::uint16_t(subtract?read_vi(source)-value:read_vi(source)+value);
        write_vi(destination,result);
    }
    void integer_add_register(unsigned destination,unsigned source,unsigned other,bool subtract=false) {
        const auto result=std::uint16_t(subtract?read_vi(source)-read_vi(other):read_vi(source)+read_vi(other));
        write_vi(destination,result);
    }
    void mtir(unsigned destination,unsigned source,unsigned lane) {
        write_vi(destination,std::uint16_t(read_vf(source,lane)));
    }
    void convert_integer(unsigned destination,unsigned source,unsigned dest,unsigned fractional_bits) {
        std::array<std::uint32_t,4> values{};
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane))values[lane]=integer_to_float(read_vf(source,lane),fractional_bits);
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane))write_vf(destination,lane,values[lane]);
    }
    void convert_fixed(unsigned destination,unsigned source,unsigned dest,unsigned fractional_bits) {
        std::array<std::uint32_t,4> values{};
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane))values[lane]=float_to_integer(read_vf(source,lane),fractional_bits);
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane))write_vf(destination,lane,values[lane]);
    }
    HG_VU_INLINE void update_arithmetic_flags(const std::array<std::uint32_t,4>& values,unsigned dest,
                                 const std::array<bool,4>& underflow,const std::array<bool,4>& overflow) {
        std::uint32_t next_mac=0,current=0;
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane)) {
            const auto bits=values[lane];
            const unsigned flags=unsigned(!(bits&0x7fffffffu))
                |(((bits&0x80000000u)&&(bits&0x7fffffffu))?2u:0u)
                |(underflow[lane]?4u:0u)|(overflow[lane]?8u:0u);
            current|=flags;
            const auto groups=(flags&1u)|((flags&2u)<<3)|((flags&4u)<<6)|((flags&8u)<<9);
            next_mac|=groups<<(3-lane);
        }
        mac=next_mac;
        status=(status&~15u)|current|(current<<6);
    }
    void multiply_acc(unsigned source,unsigned other,unsigned dest,unsigned broadcast,bool add) {
        if(dest==15&&try_vu_full_multiply_acc(*this,source,other,broadcast,add))return;
        std::array<std::uint32_t,4> values{};
        std::uint32_t next_mac=0,current=0;
        const auto scalar=read_vf(other,broadcast);
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane)) {
            const auto product=Fpu::product(read_vf(source,lane),scalar);
            auto bits=product.bits;
            bool underflow=product.underflow,overflow=product.overflow;
            if(add) {
                if(!(acc_defined&(1u<<lane)))throw std::runtime_error("VU1 MADDA reads an undefined ACC lane");
                Fpu arithmetic;bits=arithmetic.add(acc[lane],bits);
                underflow=underflow||bool(arithmetic.control&0x4000u);
                overflow=overflow||bool(arithmetic.control&0x8000u);
            }
            values[lane]=bits;
            // Collect flags while the result is live; publish only after all
            // lanes succeed, preserving the checked operation's fault state.
            const auto flags=unsigned(!(bits&0x7fffffffu))|
                (((bits&0x80000000u)&&(bits&0x7fffffffu))?2u:0u)|
                (underflow?4u:0u)|(overflow?8u:0u);
            current|=flags;
            next_mac|=((flags&1u)|((flags&2u)<<3)|((flags&4u)<<6)|((flags&8u)<<9))<<(3-lane);

        }
        mac=next_mac;status=(status&~15u)|current|(current<<6);
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane)) {acc[lane]=values[lane];acc_defined|=std::uint8_t(1u<<lane);}
    }
    void madd_vector(unsigned destination,unsigned source,unsigned other,unsigned dest,unsigned broadcast) {
        if(dest==15&&try_vu_full_madd_vector(*this,destination,source,other,broadcast))return;
        std::array<std::uint32_t,4> values{};
        std::uint32_t next_mac=0,current=0;
        const auto scalar=read_vf(other,broadcast);
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane)) {
            if(!(acc_defined&(1u<<lane)))throw std::runtime_error("VU1 MADD reads an undefined ACC lane");
            const auto product=Fpu::product(read_vf(source,lane),scalar);
            Fpu arithmetic;values[lane]=arithmetic.add(acc[lane],product.bits);
            const bool underflow=product.underflow||bool(arithmetic.control&0x4000u);
            const bool overflow=product.overflow||bool(arithmetic.control&0x8000u);
            const auto bits=values[lane];
            // Collect flags while the result is live; publish only after all
            // lanes succeed, preserving the checked operation's fault state.
            const auto flags=unsigned(!(bits&0x7fffffffu))|
                (((bits&0x80000000u)&&(bits&0x7fffffffu))?2u:0u)|
                (underflow?4u:0u)|(overflow?8u:0u);
            current|=flags;
            next_mac|=((flags&1u)|((flags&2u)<<3)|((flags&4u)<<6)|((flags&8u)<<9))<<(3-lane);
        }
        mac=next_mac;status=(status&~15u)|current|(current<<6);
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane))write_vf(destination,lane,values[lane]);
    }
    void multiply_vector(unsigned destination,unsigned source,unsigned other,unsigned dest,unsigned broadcast) {
        std::array<std::uint32_t,4> values{};
        std::array<bool,4> underflow{},overflow{};
        const auto scalar=read_vf(other,broadcast);
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane)) {
            const auto product=Fpu::product(read_vf(source,lane),scalar);
            values[lane]=product.bits;underflow[lane]=product.underflow;overflow[lane]=product.overflow;
        }
        update_arithmetic_flags(values,dest,underflow,overflow);
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane))write_vf(destination,lane,values[lane]);
    }
    void add_vector(unsigned destination,unsigned source,unsigned other,unsigned dest,bool subtract=false) {
        std::array<std::uint32_t,4> values{};std::array<bool,4> underflow{},overflow{};
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane)) {
            auto rhs=read_vf(other,lane);if(subtract)rhs^=0x80000000u;
            Fpu arithmetic;values[lane]=arithmetic.add(read_vf(source,lane),rhs);
            underflow[lane]=bool(arithmetic.control&0x4000u);overflow[lane]=bool(arithmetic.control&0x8000u);
        }
        update_arithmetic_flags(values,dest,underflow,overflow);
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane))write_vf(destination,lane,values[lane]);
    }
    void add_vector_broadcast(unsigned destination,unsigned source,unsigned other,unsigned dest,unsigned broadcast,bool subtract=false) {
        std::array<std::uint32_t,4> values{};std::array<bool,4> underflow{},overflow{};
        auto scalar=read_vf(other,broadcast);if(subtract)scalar^=0x80000000u;
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane)) {
            Fpu arithmetic;values[lane]=arithmetic.add(read_vf(source,lane),scalar);
            underflow[lane]=bool(arithmetic.control&0x4000u);overflow[lane]=bool(arithmetic.control&0x8000u);
        }
        update_arithmetic_flags(values,dest,underflow,overflow);
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane))write_vf(destination,lane,values[lane]);
    }
    void multiply_vector_lanes(unsigned destination,unsigned source,unsigned other,unsigned dest) {
        std::array<std::uint32_t,4> values{};std::array<bool,4> underflow{},overflow{};
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane)) {
            const auto product=Fpu::product(read_vf(source,lane),read_vf(other,lane));
            values[lane]=product.bits;underflow[lane]=product.underflow;overflow[lane]=product.overflow;
        }
        update_arithmetic_flags(values,dest,underflow,overflow);
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane))write_vf(destination,lane,values[lane]);
    }
    void madd_vector_lanes(unsigned destination,unsigned source,unsigned other,unsigned dest) {
        std::array<std::uint32_t,4> values{};std::array<bool,4> underflow{},overflow{};
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane)) {
            if(!(acc_defined&(1u<<lane)))throw std::runtime_error("VU1 MADD reads an undefined ACC lane");
            const auto product=Fpu::product(read_vf(source,lane),read_vf(other,lane));
            Fpu arithmetic;values[lane]=arithmetic.add(acc[lane],product.bits);
            underflow[lane]=product.underflow||bool(arithmetic.control&0x4000u);
            overflow[lane]=product.overflow||bool(arithmetic.control&0x8000u);
        }
        update_arithmetic_flags(values,dest,underflow,overflow);
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane))write_vf(destination,lane,values[lane]);
    }
    void add_acc(unsigned source,unsigned other,unsigned dest,unsigned broadcast,bool subtract=false) {
        std::array<std::uint32_t,4> values{};std::array<bool,4> underflow{},overflow{};
        auto scalar=read_vf(other,broadcast);if(subtract)scalar^=0x80000000u;
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane)) {
            Fpu arithmetic;values[lane]=arithmetic.add(read_vf(source,lane),scalar);
            underflow[lane]=bool(arithmetic.control&0x4000u);overflow[lane]=bool(arithmetic.control&0x8000u);
        }
        update_arithmetic_flags(values,dest,underflow,overflow);
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane)) {acc[lane]=values[lane];acc_defined|=std::uint8_t(1u<<lane);}
    }
    void arithmetic_q(unsigned destination,unsigned source,unsigned dest,bool multiply) {
        std::array<std::uint32_t,4> values{};
        std::array<bool,4> underflow{},overflow{};
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane)) {
            if(multiply) {
                const auto product=Fpu::product(read_vf(source,lane),q);
                values[lane]=product.bits;underflow[lane]=product.underflow;overflow[lane]=product.overflow;
            } else {
                Fpu arithmetic;values[lane]=arithmetic.add(read_vf(source,lane),q);
                underflow[lane]=bool(arithmetic.control&0x4000u);overflow[lane]=bool(arithmetic.control&0x8000u);
            }
        }
        update_arithmetic_flags(values,dest,underflow,overflow);
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane))write_vf(destination,lane,values[lane]);
    }
    void clip_test(unsigned source,unsigned other) {
        const auto w=read_vf(other,3)&0x7fffffffu;
        const auto negative_w=w|0x80000000u;
        std::uint32_t next=0;
        for(unsigned lane=0;lane!=3;++lane) {
            const auto value=read_vf(source,lane);
            if(float_less(w,value))next|=1u<<(lane*2);
            if(float_less(value,negative_w))next|=1u<<(lane*2+1);
        }
        clip=((clip<<6)|next)&0x00ffffffu;
    }
    void fcand(std::uint32_t immediate) {write_vi(1,(clip&(immediate&0x00ffffffu))?1:0);}
    void load_qword(const std::array<std::uint32_t,4096>& memory,
                    const std::array<std::uint8_t,1024>& memory_defined,unsigned ft,unsigned is,
                    std::int32_t immediate,unsigned dest) {
        if(ft>=vf.size()||is>=vi.size())throw std::runtime_error("VU1 LQ register index exceeds register file");
        const auto vector=std::int32_t(read_vi(is))+immediate;
        if(vector<0||vector>=1024)throw std::runtime_error("VU1 LQ exceeds VU1 data memory");
        if(!ft)return; // VF00 is architecturally constant.
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane)) {
            vf[ft][lane]=memory[std::size_t(vector)*4+lane];
            const auto bit=std::uint8_t(1u<<lane);
            if(memory_defined[std::size_t(vector)]&bit)vf_defined[ft]|=bit;
            else vf_defined[ft]&=std::uint8_t(~bit);
        }
    }
    void store_qword(std::array<std::uint32_t,4096>& memory,
                     std::array<std::uint8_t,1024>& memory_defined,unsigned fs,unsigned is,
                     std::int32_t immediate,unsigned dest) const {
        if(fs>=vf.size()||is>=vi.size())throw std::runtime_error("VU1 SQ register index exceeds register file");
        const auto vector=std::int32_t(read_vi(is))+immediate;
        if(vector<0||vector>=1024)throw std::runtime_error("VU1 SQ exceeds VU1 data memory");
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane)) {
            const auto bit=std::uint8_t(1u<<lane);
            const auto index=std::size_t(vector)*4+lane;
            memory[index]=fs?vf[fs][lane]:(lane==3?0x3f800000u:0u);
            if(!fs || (vf_defined[fs]&bit))memory_defined[std::size_t(vector)]|=bit;
            else memory_defined[std::size_t(vector)]&=std::uint8_t(~bit);
        }
    }
    void store_qword_increment(std::array<std::uint32_t,4096>& memory,
                               std::array<std::uint8_t,1024>& memory_defined,unsigned fs,unsigned it,
                               unsigned dest) {
        store_qword(memory,memory_defined,fs,it,0,dest);
        write_vi(it,std::uint16_t(read_vi(it)+1));
    }
    void store_integer(std::array<std::uint32_t,4096>& memory,
                       std::array<std::uint8_t,1024>& memory_defined,unsigned it,unsigned is,
                       std::int32_t immediate,unsigned dest) const {
        if(it>=vi.size()||is>=vi.size())throw std::runtime_error("VU1 ISW register index exceeds VI file");
        const auto vector=std::int32_t(read_vi(is))+immediate;
        if(vector<0||vector>=1024)throw std::runtime_error("VU1 ISW exceeds VU1 data memory");
        for(unsigned lane=0;lane!=4;++lane)if(dest&(8u>>lane)) {
            const auto index=std::size_t(vector)*4+lane;
            memory[index]=read_vi(it);
            memory_defined[std::size_t(vector)]|=std::uint8_t(1u<<lane);
        }
    }
    void load_integer(const std::array<std::uint32_t,4096>& memory,
                      const std::array<std::uint8_t,1024>& memory_defined,unsigned it,unsigned is,
                      std::int32_t immediate,unsigned dest) {
        if(it>=vi.size()||is>=vi.size())throw std::runtime_error("VU1 ILW register index exceeds VI file");
        if(dest!=8&&dest!=4&&dest!=2&&dest!=1)
            throw std::runtime_error("VU1 ILW requires exactly one destination field");
        const auto vector=std::int32_t(read_vi(is))+immediate;
        if(vector<0||vector>=1024)throw std::runtime_error("VU1 ILW exceeds VU1 data memory");
        const unsigned lane=dest==8?0:dest==4?1:dest==2?2:3;
        if(!(memory_defined[std::size_t(vector)]&(1u<<lane)))
            throw std::runtime_error("VU1 ILW reads undefined VU1 data memory");
        write_vi(it,std::uint16_t(memory[std::size_t(vector)*4+lane]));
    }
};

// Merge one concurrently executed VU snapshot and one live VIF feeder.
// Only VIF writes need instrumentation: work starts from the same activation
// baseline and is the authoritative VU result. Resolve possible conflicts on
// VIF-touched locations, fold VIF-only changes into work, then bulk-copy work
// back to live. This keeps all VU store instructions on their normal hot path.
inline void merge_vu1_overlap_memory(
        std::array<std::uint32_t,4096>& live,
        std::array<std::uint8_t,1024>& live_defined,
        std::array<std::uint32_t,4096>& work,
        std::array<std::uint8_t,1024>& work_defined,
        const Vu1MemoryWriteTracker& vif_writes) {
    for(const auto raw:vif_writes.word_indices) {
        const auto index=std::size_t(raw);
        const auto baseline=vif_writes.baseline_words[index];
        const auto live_value=live[index],work_value=work[index];
        const bool live_changed=live_value!=baseline,work_changed=work_value!=baseline;
        if(live_changed&&work_changed&&live_value!=work_value)
            throw std::runtime_error("VU1 overlap memory conflict");
        if(live_changed)work[index]=live_value;
    }
    for(const auto raw:vif_writes.vector_indices) {
        const auto vector=std::size_t(raw);
        const auto baseline=vif_writes.baseline_defined[vector];
        const auto live_mask=live_defined[vector],work_mask=work_defined[vector];
        std::uint8_t merged=baseline;
        for(unsigned lane=0;lane<4;++lane) {
            const auto bit=std::uint8_t(1u<<lane);
            const bool base=(baseline&bit)!=0,l=(live_mask&bit)!=0,w=(work_mask&bit)!=0;
            const bool live_changed=l!=base,work_changed=w!=base;
            if(live_changed&&work_changed&&l!=w)
                throw std::runtime_error("VU1 overlap memory conflict");
            const bool value=live_changed?l:work_changed?w:base;
            if(value)merged|=bit;else merged&=std::uint8_t(~bit);
        }
        work_defined[vector]=merged;
    }
    live=work;
    live_defined=work_defined;
}

// Bounded VIF1 transport for the documented VIFcode stream. It implements only
// state-only codes, VU1 memory writes, microprogram uploads, and PATH2
// DIRECT/DIRECTHL. Microprogram activation dispatches only to build-time AOT
// bodies supplied by the generated translation; unknown programs remain faults.
struct Vif1Path {
    std::vector<std::uint8_t> pending;
    // Physical byte phase of pending[0]. TTE contributes only a DMAtag's upper
    // 64 bits at phase 8, so a retained VIF command can cross a discontinuity
    // between ordinary qword data (next phase 0) and transferred tag bytes.
    // Keep sparse breakpoints rather than a phase byte beside every payload byte.
    struct PendingPhaseBreak {std::size_t offset=0;std::uint8_t phase=0;};
    std::size_t pending_phase=0;
    std::vector<PendingPhaseBreak> pending_phase_breaks;
    std::size_t pending_byte_phase(std::size_t at) const {
        if(at>pending.size())throw std::runtime_error("VIF1 pending phase offset exceeds retained bytes");
        for(auto it=pending_phase_breaks.rbegin();it!=pending_phase_breaks.rend();++it)
            if(at>=it->offset)return (std::size_t(it->phase)+at-it->offset)&15u;
        return (pending_phase+at)&15u;
    }
    void begin_pending_source(std::size_t phase) {
        phase&=15u;
        const auto offset=pending.size();
        if(!offset) {pending_phase=phase;pending_phase_breaks.clear();return;}
        if(pending_byte_phase(offset)!=phase)
            pending_phase_breaks.push_back({offset,std::uint8_t(phase)});
    }
    void consume_pending_prefix(std::size_t count) {
        if(count>pending.size())throw std::runtime_error("VIF1 consumed beyond retained bytes");
        if(!count)return;
        const auto remaining=pending.size()-count;
        const auto phase=pending_byte_phase(count);
        pending.erase(pending.begin(),pending.begin()+count);
        if(!remaining) {pending_phase=phase;pending_phase_breaks.clear();return;}
        std::size_t output=0;
        for(const auto& item:pending_phase_breaks)if(item.offset>count)
            pending_phase_breaks[output++]={item.offset-count,item.phase};
        pending_phase_breaks.resize(output);pending_phase=phase;
    }
    std::array<std::uint32_t,4> row{}, column{};
    // VU1 data memory is 16 KiB / 1024 128-bit vectors. UNPACK writes here;
    // it never implies VU microprogram execution.
    std::array<std::uint32_t,4096> vu_mem{};
    // One bit per lane (bit0=X ... bit3=W). Initialize existing modeled memory
    // as defined and mark only Sony-documented V2/V3 trailing outputs undefined.
    std::array<std::uint8_t,1024> vu_mem_defined{};
    // VU1 MicroMem is 16 KiB / 2048 64-bit instruction pairs. MPG only loads
    // the checked storage. A generated AOT body validates the uploaded identity
    // before executing a configured entry point.
    // Kept heap-backed so VIF endpoints remain safe to instantiate on the
    // host stack while retaining their fixed, checked hardware capacity.
    std::vector<std::uint64_t> vu_micro_mem=std::vector<std::uint64_t>(2048);
    Vu1State vu1;
    Vu1AotExecutor vu1_executor=nullptr;
    // Host scheduling only. Disabled by default so tests/other runners retain the
    // exact synchronous VIF->VU behavior. The native game enables it only while
    // draining VIF1 DMA, where the helper feeder can fill the alternate buffer.
    bool host_overlap_enabled=false;
    bool host_vu_pending=false;
    bool host_vu_running=false;
    bool host_vu_stalled=false;
    std::uint16_t host_vu_entry=0;
    // Installed only while the native overlap feeder is writing live VU data.
    Vu1MemoryWriteTracker* host_vif_memory_tracker=nullptr;
    // Host invalidation token for the private AOT snapshot's MicroMem copy.
    std::uint64_t host_micro_generation=0;
    std::uint32_t cycle=0,offset=0,base=0,itops=0,itop=0,tops=0,top=0,mode=0,mark=0,mask=0;
    bool dbf=false;
    bool path3_masked=false;
    bool mark_detected=false;
    static constexpr std::size_t max_pending=1024*1024+16;
    std::uint32_t error_mask=0;
    Vif1Path() {vu_mem_defined.fill(0x0fu);}
    static bool register_contains(std::uint32_t address) {return address>=0x10003c00u && address<0x10003e00u;}
    void reset_interface() {
        pending.clear();pending_phase=0;pending_phase_breaks.clear();row={};column={};
        cycle=offset=base=itops=itop=tops=top=mode=mark=mask=error_mask=0;
        host_vu_pending=host_vu_running=host_vu_stalled=false;host_vu_entry=0;
        dbf=false;
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
        case 0x02: // OFFSET clears DBF; BASE becomes both current and next buffer origin.
            offset=immediate&0x3ff;dbf=false;top=base;tops=base;return;
        case 0x03:base=immediate&0x3ff;return; // BASE
        case 0x04:itops=immediate&0x3ff;return; // ITOP VIFcode writes ITOPS.
        case 0x05:mode=immediate&3;if(mode==3)throw std::runtime_error("reserved VIF1 STMOD mode");return; // STMOD
        case 0x06:path3_masked=(immediate&0x8000)!=0;return; // MSKPATH3
        case 0x07:mark=immediate;mark_detected=true;return; // MARK
        case 0x10:case 0x11:case 0x13:return; // FLUSHE/FLUSH/FLUSHA: synchronous host boundary
        case 0x14:throw std::runtime_error("VIF1 MSCAL must be processed with the AOT VU1 dispatcher");
        case 0x15:throw std::runtime_error("VIF1 MSCALF requires unavailable AOT VU1 execution");
        case 0x17:throw std::runtime_error("VIF1 MSCNT requires unavailable AOT VU1 execution");
        default:throw std::runtime_error("unsupported VIF1 state VIFcode");
        }
    }
    void activate_vu1_state() {
        // EE User's Manual v6.0: activation copies ITOPS->ITOP and TOPS->TOP,
        // reverses DBF, then prepares the other double buffer in TOPS.
        itop=itops;
        top=tops;
        dbf=!dbf;
        tops=(dbf?base+offset:base)&0x3ffu;
    }
    void activate_mscal(std::uint16_t immediate,GifPath& gif,GsRegisterState& gs) {
        activate_vu1_state();
        if(!vu1_executor)throw std::runtime_error("VIF1 MSCAL requires unavailable AOT VU1 execution");
        const auto entry=std::uint16_t((immediate&0x3ffu)*8u);
        if(host_overlap_enabled) {
            if(host_vu_running||host_vu_pending)throw std::runtime_error("VIF1 MSCAL reached an unjoined VU1 host job");
            host_vu_entry=entry;host_vu_pending=true;return;
        }
        vu1_executor(*this,entry,gif,gs);
    }
    void activate_mscnt(GifPath& gif,GsRegisterState& gs) {
        // EE User's Manual v6.0: MSCNT performs the same activation-state
        // transfers as MSCAL/MSCALF, then continues from the PC following the
        // most recently ended microprogram.
        activate_vu1_state();
        if(!vu1_executor)throw std::runtime_error("VIF1 MSCNT requires unavailable AOT VU1 execution");
        const auto entry=vu1.tpc;
        if(host_overlap_enabled) {
            if(host_vu_running||host_vu_pending)throw std::runtime_error("VIF1 MSCNT reached an unjoined VU1 host job");
            host_vu_entry=entry;host_vu_pending=true;return;
        }
        vu1_executor(*this,entry,gif,gs);
    }
    void xgkick(unsigned is,GifPath& gif,GsRegisterState& gs) {
        if(!gif.pending.empty())throw std::runtime_error("VU1 XGKICK requires idle bounded GIF packet state");
        auto vector=std::size_t(vu1.read_vi(is));
        if(vector>=1024)throw std::runtime_error("VU1 XGKICK starts outside VU1 data memory");
        bool expect_tag=true,eop=false;
        unsigned format=0;
        std::size_t nreg=0,entries_remaining=0,entry_index=0;
        std::uint64_t descriptors=0;
        const auto packed_required_mask=[](unsigned descriptor) {
            // PACKED payload fields consumed by GsRegisterState::write_packed.
            // Undefined lanes that the selected descriptor never reads need not
            // be invented merely because XGKICK physically transfers 128 bits.
            switch(descriptor) {
            case 0x0:return 0x1u; // PRIM low 11 bits.
            case 0x1:return 0xfu; // RGBA uses one byte from every word.
            case 0x2:return 0x7u; // STQ consumes S,T,Q; fourth word is padding.
            case 0x3:return 0x3u; // UV consumes the low two words.
            case 0x4:case 0x5:return 0xfu; // XYZF/XYZ including ADC.
            case 0xa:return 0x8u; // FOG lives in the upper word.
            case 0xc:case 0xd:return 0x3u; // Direct XYZF3/XYZ3 low64 form.
            case 0xe:return 0x7u; // A+D: low64 data + low byte of high64 address.
            case 0xf:return 0x0u; // NOP descriptor.
            default:return 0xfu; // Preserve strict validation for unsupported forms.
            }
        };
        for(std::size_t count=0;vector<1024;++vector,++count) {
            const auto base=vector*4;
            const auto low=std::uint64_t(vu_mem[base])|(std::uint64_t(vu_mem[base+1])<<32);
            const auto high=std::uint64_t(vu_mem[base+2])|(std::uint64_t(vu_mem[base+3])<<32);
            const auto defined=unsigned(vu_mem_defined[vector]);
            if(expect_tag) {
                if(defined!=0x0f)throw std::runtime_error("VU1 XGKICK reads undefined GIF tag");
                const auto nloop=std::size_t(low&0x7fffu);
                eop=(low&(1ull<<15))!=0;
                format=unsigned((low>>58)&3u);
                const auto encoded_nreg=std::size_t((low>>60)&0xfu);
                nreg=encoded_nreg?encoded_nreg:16;
                descriptors=high;
                entry_index=0;
                entries_remaining=format>=2?nloop:nloop*nreg;
            } else {
                unsigned required=0x0f;
                if(format==0) {
                    const auto descriptor=unsigned((descriptors>>((entry_index%nreg)*4))&0xfu);
                    required=packed_required_mask(descriptor);
                } else if(format==1) {
                    required=0;
                    const auto first=unsigned((descriptors>>((entry_index%nreg)*4))&0xfu);
                    if(first!=0xf)required|=0x3;
                    if(entries_remaining>1) {
                        const auto second=unsigned((descriptors>>(((entry_index+1)%nreg)*4))&0xfu);
                        if(second!=0xf)required|=0xc;
                    }
                }
                if((defined&required)!=required)
                    throw std::runtime_error("VU1 XGKICK reads undefined data required by GIF descriptor");
            }
            gif.submit_qword(low,high,gs);
            if(expect_tag) {
                if(!entries_remaining) {
                    if(eop) {if(gif.pending.empty())return;throw std::runtime_error("VU1 XGKICK EOP tag left incomplete GIF state");}
                    expect_tag=true;
                } else expect_tag=false;
                continue;
            }
            const auto consumed=format==1&&entries_remaining>1?std::size_t(2):std::size_t(1);
            entries_remaining-=consumed;entry_index+=consumed;
            if(!entries_remaining) {
                expect_tag=true;
                if(eop) {if(gif.pending.empty())return;throw std::runtime_error("VU1 XGKICK EOP payload left incomplete GIF state");}
            }
        }
        throw std::runtime_error("VU1 XGKICK packet exceeds VU1 data memory");
    }
    void submit_qword(std::uint64_t low,std::uint64_t high,GifPath& gif,GsRegisterState& gs) {
        begin_pending_source(0);
        const auto offset=pending.size();
        pending.resize(offset+16);
        for(unsigned i=0;i!=8;++i) {
            pending[offset+i]=std::uint8_t(low>>(i*8));
            pending[offset+8+i]=std::uint8_t(high>>(i*8));
        }
        process_pending(gif,gs);
    }
    void submit_dma_tag(std::uint64_t high,GifPath& gif,GsRegisterState& gs) {
        // EE manual p86: the low 64 bits are DMAtag, only the upper two words
        // enter the VIFcode/data stream, beginning at physical byte phase 8.
        // Retained bytes may therefore be logically adjacent but physically
        // discontinuous; begin_pending_source records that boundary sparsely.
        begin_pending_source(8);
        for(unsigned i=0;i!=8;++i)pending.push_back(std::uint8_t(high>>(i*8)));
        process_pending(gif,gs);
    }
    void process_pending(GifPath& gif,GsRegisterState& gs);
};

} // namespace hg
#undef HG_VU_INLINE
