"""Build-time structural decoder for VU microinstruction pairs.

This is analysis infrastructure for an AOT VU translator, never a runtime VU
interpreter.  It intentionally recognizes only the lower-pipeline control-flow
forms needed to build a checked CFG; arithmetic remains unimplemented.
"""

from collections import Counter
from dataclasses import asdict, dataclass


_CONTROL={
    0x20:'b', 0x21:'bal', 0x24:'jr', 0x25:'jalr',
    0x28:'ibeq', 0x29:'ibne', 0x2d:'ibgtz', 0x2e:'iblez', 0x2f:'ibgez',
}
_DIRECT_BRANCH={'b','bal','ibeq','ibne','ibgtz','iblez','ibgez'}


def _signed(value,bits):
    return value-(1<<bits) if value&(1<<(bits-1)) else value


@dataclass(frozen=True)
class VuPair:
    address: int
    lower: int
    upper: int
    lower_opcode: int
    upper_opcode: int
    end: bool
    lower_is_immediate: bool
    lower_name: str
    branch_target: int | None

    def report(self):
        return asdict(self)


def decode_pair(address,word):
    """Decode one little-endian 64-bit LIW pair at an eight-byte address."""
    if type(address) is not int or address<0 or address%8:
        raise ValueError('VU instruction-pair address must be nonnegative and 8-byte aligned')
    if type(word) is not int or not 0<=word<1<<64:
        raise ValueError('VU instruction pair must be an unsigned 64-bit value')
    lower=word&0xffffffff
    upper=word>>32
    immediate=bool(upper&0x80000000)
    end=bool(upper&0x40000000)
    opcode=lower>>25
    name='immediate' if immediate else _CONTROL.get(opcode,'other')
    target=None
    if name in _DIRECT_BRANCH:
        # The signed offset is measured in 64-bit instruction pairs. The
        # documented branch base is the pair following the delay pair (+8).
        target=address+8+_signed(lower&0x7ff,11)*8
        if target<0:
            raise ValueError('VU branch target precedes address zero')
    return VuPair(address,lower,upper,opcode,upper&0x3f,end,immediate,name,target)


def decode_program(data,base=0):
    """Decode raw MicroMem bytes without executing or translating them."""
    if type(data) is not bytes or len(data)%8:
        raise ValueError('VU microprogram must contain complete 64-bit instruction pairs')
    if type(base) is not int or base<0 or base%8:
        raise ValueError('VU microprogram base must be nonnegative and 8-byte aligned')
    return [decode_pair(base+offset,int.from_bytes(data[offset:offset+8],'little'))
            for offset in range(0,len(data),8)]


def report_program(data,base=0):
    pairs=decode_program(data,base)
    addresses={pair.address for pair in pairs}
    upper_counts=Counter(pair.upper_opcode for pair in pairs)
    lower_counts=Counter(pair.lower_opcode for pair in pairs)
    edges=[{'source':pair.address,'delay_slot':pair.address+8,'target':pair.branch_target,'kind':pair.lower_name,
            'delay_slot_in_program':pair.address+8 in addresses,'in_program':pair.branch_target in addresses}
           for pair in pairs if pair.branch_target is not None]
    return {'pair_count':len(pairs),'base':base,
            'end_pairs':[pair.address for pair in pairs if pair.end],
            'control_pairs':[pair.report() for pair in pairs if pair.lower_name in _CONTROL.values()],
            'direct_edges':edges,
            'missing_branch_delay_slots':sorted({edge['delay_slot'] for edge in edges if not edge['delay_slot_in_program']}),
            'external_direct_targets':sorted({edge['target'] for edge in edges if not edge['in_program']}),
            'upper_opcode_counts':[{'opcode':opcode,'count':count} for opcode,count in sorted(upper_counts.items())],
            'lower_opcode_counts':[{'opcode':opcode,'count':count} for opcode,count in sorted(lower_counts.items())],
            'note':'Structural VU pair/control-flow analysis only; no VU execution or interpreter fallback.'}
