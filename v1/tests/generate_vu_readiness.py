"""Generate redistributable baseline/optimized VU scheduling comparisons."""
from pathlib import Path
import sys

from hgtool.vu_decode import decode_pair
from hgtool.vu_emit import _body_lines
from test_vu_decode import pair, upper_direct, lower_primary, lower_special, lower_imm15

NOP=0x8000033c


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
    return [(linear,[0]),(branch,[0,24]),(loop,[0]),(integer,[0,16])]


def generate():
    out=['#include "hg/vif.hpp"','#include "hg/vu_xyz.hpp"','#include "hg/vu_specialized.hpp"',
         '#include <iostream>','#include <tuple>','#include <string>']
    for number,(words,entries) in enumerate(programs()):
        pairs=[decode_pair(i*8,word) for i,word in enumerate(words)]
        for fast in (False,True):
            out.append(f'static void run_{number}_{int(fast)}(hg::Vif1Path& v,unsigned entry) {{')
            if fast:out.append('bool vu_ready_clock_safe=true;')
            out += [f'if(entry=={entry})goto L_{entry:04x};' for entry in entries]
            out.append('throw std::runtime_error("synthetic entry");')
            body=_body_lines(pairs,entries,optimize_readiness=fast)
            if fast:assert any('if(!vu_ready_clock_safe)' in line for line in body)
            out+=body+['}']
    out.append(r'''
int main(){
    using Run=void(*)(hg::Vif1Path&,unsigned);
    const Run functions[4][2]={{run_0_0,run_0_1},{run_1_0,run_1_1},{run_2_0,run_2_1},{run_3_0,run_3_1}};
    std::uint32_t random=0x1a39b751u;unsigned checked=0;
    const auto next=[&](){random^=random<<13;random^=random>>17;random^=random<<5;return random;};
    const auto state=[](const hg::Vu1State& s){return std::tie(s.vf,s.vf_defined,s.vi,s.acc,s.acc_defined,
        s.clip,s.status,s.mac,s.q,s.i,s.p,s.pending_q,s.pending_p,s.q_cycles_remaining,s.p_cycles_remaining,
        s.q_pending,s.p_pending,s.tpc,s.issue_cycle,s.vf_ready,s.vi_ready);};
    hg::Vif1Path initial;
    for(unsigned program=0;program<4;++program)for(unsigned sample=0;sample<16384;++sample){
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
    std::cout<<checked<<" compiled VU readiness comparisons passed\n";return 0;
}
''')
    return '\n'.join(out)+'\n'


if __name__=='__main__':
    Path(sys.argv[1]).write_text(generate(),encoding='utf-8')
