"""Generate redistributable baseline/optimized VU scheduling comparisons."""
from pathlib import Path
import sys

from hgtool.vu_decode import decode_pair
from hgtool.vu_emit import _body_lines
from test_vu_decode import pair, upper_direct, upper_special, lower_primary, lower_special, lower_imm15

NOP=0x8000033c



def transform_program(unsigned_immediate=False):
    # Synthetic register allocation/strides/constants; no original game bytes.
    increment=lambda reg: lower_imm15(8,4,it=reg,is_=reg) if unsigned_immediate else 0x80000032|(4<<6)|(reg<<11)|(reg<<16)
    return [
        pair(lower_primary(0,it=14,is_=2,imm11=1,dest=15),upper_special(0x1f,fs=14,ft=14)),
        pair(lower_primary(0,it=15,is_=2,imm11=2,dest=15)),
        pair(lower_imm15(9,1,it=8,is_=8)),
        pair(lower_primary(1,is_=25,it=7,imm11=-2,dest=14),upper_direct(0x1c,fs=16,fd=25,dest=14)),
        pair(lower_primary(1,is_=26,it=7,imm11=-4,dest=14),upper_special(0x18,fs=17,ft=14,dest=15)),
        pair(lower_primary(0,it=26,is_=2,imm11=-1,dest=14),upper_special(9,fs=18,ft=14,dest=15)),
        pair((0x12<<25)|0x654321,upper_special(10,fs=19,ft=14,dest=15)),
        pair(0x80000030|(1<<6)|(9<<11)|(1<<16),upper_direct(0x0b,fs=20,ft=0,fd=16,dest=15)),
        pair(lower_special(0x3c,is_=14,it=9,fsf=3),upper_special(0x18,fs=21,ft=14,dest=15)),
        pair(lower_primary(1,is_=15,it=7,imm11=2,dest=15),upper_special(9,fs=22,ft=14,dest=15)),
        pair(lower_primary(5,it=1,is_=7,imm11=-3,dest=1),upper_special(10,fs=23,ft=14,dest=15)),
        pair(lower_special(0x38,is_=0,it=16,fsf=3,ftf=3),upper_direct(0x0b,fs=24,ft=0,fd=14,dest=15)),
        pair(increment(2),upper_direct(0x1c,fs=26,fd=26,dest=14)),
        pair(lower_primary(0x29,is_=0,it=8,imm11=-14),upper_special(0x15,fs=25,ft=25,dest=12)),
        pair(increment(7),upper_special(0x14,fs=25,ft=25,dest=2)),
        pair(NOP,upper_special(0x2f,flags=0x40000000)),pair(NOP),
    ]

def programs():
    linear=[]
    for i in range(12):
        lower=NOP
        source=8 if i in (5,9) else 1
        upper=upper_direct(0x28,dest=14 if i&1 else 15,fs=source,ft=2,fd=3,
                           flags=0x40000000 if i==11 else 0)
        if i==2:lower=lower_special(0x3b)
        if i==4:lower=lower_primary(0,dest=14,it=8,is_=4)
        if i==6:lower=lower_special(0x38,is_=1,it=2)
        if i==7:upper=upper_direct(0x20,dest=14,fs=1,fd=3)
        if i==10:lower=lower_primary(1,dest=14,it=4,is_=3)
        linear.append(pair(lower,upper))
    linear.append(pair(NOP))
    branch=[
        pair(NOP,upper_direct(0x28,dest=15,fs=1,ft=2,fd=3)),
        pair(lower_primary(0x28,is_=1,it=0,imm11=2)),
        pair(NOP,upper_direct(0x28,dest=8,fs=1,ft=2,fd=1)),
        pair(NOP,upper_direct(0x28,dest=15,fs=1,ft=2,fd=6)),
        pair(NOP,upper_direct(0x28,dest=15,fs=1,ft=2,fd=3)),
        pair(NOP,upper_direct(0x28,dest=14,fs=1,ft=2,fd=3,flags=0x40000000)),
        pair(NOP),
    ]
    loop=[
        pair(NOP,upper_direct(0x28,dest=15,fs=1,ft=2,fd=3)),
        pair(lower_imm15(0x09,1,it=2,is_=2),upper_direct(0x28,dest=14,fs=1,ft=2,fd=4)),
        pair(lower_primary(0x29,is_=2,it=0,imm11=-3),upper_direct(0x18,dest=14,fs=3,ft=1,fd=5)),
        pair(NOP,upper_direct(0x28,dest=15,fs=1,ft=2,fd=6)),
        pair(NOP,upper_direct(0x28,dest=15,fs=1,ft=2,fd=3,flags=0x40000000)),
        pair(lower_primary(1,dest=14,it=4,is_=3),upper_direct(0x28,dest=14,fs=3,ft=1,fd=3)),
    ]
    # Integer readiness: one-cycle writes, repeated address reads, four-cycle
    # ILW producers, immediate consumers, fully elapsed consumers and re-entry.
    integer=[
        pair(lower_imm15(0x08,2,it=4,is_=0)),
        pair(lower_primary(0,dest=14,it=8,is_=4)),
        pair(lower_primary(0,dest=14,it=9,is_=4,imm11=1)),
        pair(lower_primary(4,dest=8,it=6,is_=4)),
        pair(lower_imm15(0x08,1,it=7,is_=6)),
        pair(lower_imm15(0x08,1,it=8,is_=7)),
        pair(lower_primary(5,dest=8,it=8,is_=4,imm11=2)),
        pair(lower_primary(4,dest=8,it=6,is_=4,imm11=1)),
        pair(NOP),pair(NOP),pair(NOP),
        pair(lower_imm15(0x08,1,it=7,is_=6)),
        pair(lower_primary(0,dest=14,it=10,is_=4),
             upper_direct(0x28,dest=15,fs=1,ft=2,fd=3,flags=0x40000000)),
        pair(lower_imm15(0x08,1,it=8,is_=7)),
    ]
    return [(linear,[0]),(branch,[0,24]),(loop,[0]),(integer,[0,16]),(transform_program(),[0])]


def generate():
    out=['#include "hg/vif.hpp"','#include "hg/vu_xyz.hpp"','#include "hg/vu_specialized.hpp"',
         '#include <iostream>','#include <tuple>','#include <string>','static unsigned static_loop_hits=0;']
    out.append(r'''
#include "hg/vu_matrix.hpp"
#include "hg/fpu_add4.hpp"
#if defined(_M_X64) || defined(__x86_64__)
#include <immintrin.h>
#endif
namespace hg {
bool scalar_transform_disabled(const std::array<const std::uint32_t*,8>&,const std::array<std::uint32_t,4>&,VuTransformResult&){return false;}
}
#define try_fpu_transform scalar_transform_disabled
#define fused_vu_transform scalar_fused_vu_transform
#define prepare_vu_matrix scalar_prepare_vu_matrix
#define commit_vu_matrix scalar_commit_vu_matrix
#include "@MATRIX_SOURCE@"
#undef try_fpu_transform
#undef fused_vu_transform
#undef prepare_vu_matrix
#undef commit_vu_matrix
static void check_fused_transform(){
    std::uint32_t random=0x923a65c1u;
    const auto next=[&](){random^=random<<13;random^=random>>17;random^=random<<5;return random;};
#if defined(_M_X64) || defined(__x86_64__)
    const auto saved=_mm_getcsr();
#endif
    for(unsigned mode=0;mode<16;++mode){
#if defined(_M_X64) || defined(__x86_64__)
        _mm_setcsr((saved&~0xe07fu)|((mode&3)<<13)|((mode&4)?0x8000:0)|((mode&8)?0x40:0)|0x1f80);
#endif
        for(unsigned sample=0;sample<16384;++sample){
            hg::Vu1State state;
            hg::VuTransformSources sources{};
            for(unsigned i=0;i<8;++i){state.vf_defined[i+1]=15;for(auto& lane:state.vf[i+1])lane=next();sources[i]=state.vf[i+1].data();}
            state.vf_defined[11]=15;for(auto& lane:state.vf[11])lane=next();
            if(sample%4==0)for(auto& lane:state.vf[11])lane=(lane&0x807fffff)|0x3f800000;
            if(sample%8==0)for(unsigned i=1;i<=8;++i)for(auto& lane:state.vf[i])lane=(lane&0x807fffff)|0x3f000000;
            const auto actual=hg::fused_vu_transform(sources,state.vf[11]);
            const auto scalar=hg::scalar_fused_vu_transform(sources,state.vf[11]);
            for(unsigned chain=0;chain<2;++chain){
                state.multiply_acc(chain*4+1,11,15,0,false);
                state.multiply_acc(chain*4+2,11,15,1,true);
                state.multiply_acc(chain*4+3,11,15,2,true);
                state.madd_vector(16+chain,chain*4+4,0,15,3);
            }
            if(actual.values[0]!=state.vf[16]||actual.values[1]!=state.vf[17]||actual.acc!=state.acc||
               actual.mac!=state.mac||actual.current!=(state.status&15)||actual.sticky!=((state.status>>6)&15)||
               actual.values!=scalar.values||actual.acc!=scalar.acc||actual.mac!=scalar.mac||
               actual.current!=scalar.current||actual.sticky!=scalar.sticky)
                throw std::runtime_error("fused transform scalar/reference mismatch");
        }
    }
#if defined(_M_X64) || defined(__x86_64__)
    _mm_setcsr(saved);
#endif
    std::cout<<"262144 fused transform scalar/reference comparisons passed\n";
}
'''.replace('@MATRIX_SOURCE@',(Path(__file__).resolve().parents[1]/'runtime/vu_matrix.cpp').as_posix()))
    for number,(words,entries) in enumerate(programs()):
        pairs=[decode_pair(i*8,word) for i,word in enumerate(words)]
        for fast in (False,True):
            out.append(f'static void run_{number}_{int(fast)}(hg::Vif1Path& v,unsigned entry) {{')
            if fast:out.append('bool vu_ready_clock_safe=true;')
            out += [f'if(entry=={entry})goto L_{entry:04x};' for entry in entries]
            out.append('throw std::runtime_error("synthetic entry");')
            body=_body_lines(pairs,entries,optimize_readiness=fast,optimize_static_loop=fast)
            if number==4 and fast:
                assert any('Static whole-loop schedule' in line for line in body)
                body=[line.replace('        v.vu1.advance_pipeline(4);','        ++static_loop_hits;v.vu1.advance_pipeline(4);') for line in body]
            if fast:assert any('if(!vu_ready_clock_safe)' in line for line in body)
            out+=body+['}']
    out.append(r'''
int main(){
    check_fused_transform();
    using Run=void(*)(hg::Vif1Path&,unsigned);
    const Run functions[5][2]={{run_0_0,run_0_1},{run_1_0,run_1_1},{run_2_0,run_2_1},{run_3_0,run_3_1},{run_4_0,run_4_1}};
    std::uint32_t random=0x1a39b751u;unsigned checked=0;
    const auto next=[&](){random^=random<<13;random^=random>>17;random^=random<<5;return random;};
    const auto state=[](const hg::Vu1State& s){return std::tie(s.vf,s.vf_defined,s.vi,s.acc,s.acc_defined,
        s.clip,s.status,s.mac,s.q,s.i,s.p,s.pending_q,s.pending_p,s.q_cycles_remaining,s.p_cycles_remaining,
        s.q_pending,s.p_pending,s.tpc,s.issue_cycle,s.vf_ready,s.vi_ready);};
    hg::Vif1Path initial;
    for(unsigned program=0;program<5;++program)for(unsigned sample=0;sample<16384;++sample){
        auto& s=initial.vu1;s=hg::Vu1State{};
        s.issue_cycle=sample%4?next()%32:~std::uint64_t(0)-(next()%16);
        for(unsigned reg=0;reg<32;++reg){
            s.vf_defined[reg]=std::uint8_t(sample%7?15:next());
            for(unsigned lane=0;lane<4;++lane){s.vf[reg][lane]=next();
                s.vf_ready[reg][lane]=sample%8?s.issue_cycle+next()%16:~std::uint64_t(0);}
        }
        for(unsigned reg=0;reg<16;++reg){s.vi[reg]=std::uint16_t(next());s.vi_ready[reg]=s.issue_cycle+next()%16;}
        s.vi[1]=(sample>>1)&1;s.vi[2]=1+sample%4;s.vi[4]=sample%31?2:65535;
        for(auto& value:s.acc)value=next();s.acc_defined=sample%11?15:next();
        s.status=next();s.mac=next();s.clip=next();s.i=next();s.q=next();s.p=next();
        s.pending_q=next();s.pending_p=next();s.q_pending=sample&1;s.p_pending=sample&2;
        s.q_cycles_remaining=sample%13?next()%16:0xffffffffu;s.p_cycles_remaining=next()%16;
        for(auto& value:initial.vu_mem)value=next();
        for(auto& value:initial.vu_mem_defined)value=std::uint8_t(sample%11?15:next());
        if(program==4) {
            s.vi[2]=16;s.vi[7]=512;s.vi[8]=1+sample%5;
            if(sample%2==0) {
                s.issue_cycle=100;
                for(auto& ready:s.vf_ready)ready.fill(0);
                s.vi_ready.fill(0);s.vf_defined.fill(15);initial.vu_mem_defined.fill(15);
                s.q_cycles_remaining=sample%13;
            }
            // Real faults/guard rejections, aliases, wrap, resource stalls,
            // and departure from the fast path after several good iterations.
            switch(sample%23) {
            case 0:s.vi[2]=1022;break;
            case 1:s.vi[7]=2;break;
            case 2:s.vi[7]=s.vi[2]+3;break;
            case 3:initial.vu_mem_defined[s.vi[2]+1]=7;break;
            case 4:s.vf_defined[17]=0;break;
            case 5:s.issue_cycle=~std::uint64_t(0)-10;break;
            case 6:s.q_pending=true;s.q_cycles_remaining=50;break;
            case 7:s.vi[2]=1016;s.vi[8]=4;break;
            case 8:s.vi[7]=1016;s.vi[8]=4;break;
            case 9:s.vf_ready[14][0]=s.issue_cycle+2;break;
            case 10:s.vi_ready[2]=s.issue_cycle+2;break;
            case 11:s.q_pending=true;s.q_cycles_remaining=0;break;
            case 12:s.p_pending=true;s.p_cycles_remaining=0;break;
            }
        }
        auto reference=initial,actual=initial;
        const unsigned entry=(sample&1)?(program==1?24:program==3?16:0):0;std::string first,second;
        try{functions[program][0](reference,entry);}catch(const std::exception& e){first=e.what();}
        try{functions[program][1](actual,entry);}catch(const std::exception& e){second=e.what();}
        if(first!=second||state(reference.vu1)!=state(actual.vu1)||reference.vu_mem!=actual.vu_mem||
           reference.vu_mem_defined!=actual.vu_mem_defined){
            std::cerr<<"VU readiness mismatch program="<<program<<" sample="<<sample
                     <<" reference="<<first<<" actual="<<second<<'\n';return 1;}
        ++checked;
    }
    if(static_loop_hits<1000){std::cerr<<"Insufficient static loop coverage: "<<static_loop_hits;return 1;}
    std::cout<<checked<<" compiled VU readiness comparisons passed; static loop iterations="<<static_loop_hits<<"\n";return 0;
}
''')
    return '\n'.join(out)+'\n'


if __name__=='__main__':
    Path(sys.argv[1]).write_text(generate(),encoding='utf-8')
