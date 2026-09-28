"""Build-time structural decoder for VU microinstruction pairs.

This is analysis infrastructure for an AOT VU translator, never a runtime VU
interpreter.  It intentionally recognizes only the lower-pipeline control-flow
forms needed to build a checked CFG; arithmetic remains unimplemented.
"""

from collections import Counter
from dataclasses import asdict, dataclass


_LOWER_PRIMARY={
    0x00:'lq', 0x01:'sq', 0x04:'ilw', 0x05:'isw',
    0x08:'iaddiu', 0x09:'isubiu',
    0x11:'fcset', 0x12:'fcand',
    0x20:'b', 0x21:'bal', 0x24:'jr', 0x25:'jalr',
    0x28:'ibeq', 0x29:'ibne', 0x2c:'ibltz', 0x2d:'ibgtz',
    0x2e:'iblez', 0x2f:'ibgez',
}
_LOWER_DIRECT={0x30:'iadd',0x31:'isub',0x32:'iaddi',0x34:'iand',0x35:'ior'}
_LOWER_SPECIAL={
    0x30:'move',0x31:'mr32',0x34:'lqi',0x35:'sqi',0x36:'lqd',0x37:'sqd',
    0x38:'div',0x39:'sqrt',0x3a:'rsqrt',0x3b:'waitq',0x3c:'mtir',0x3d:'mfir',
    0x3e:'ilwr',0x3f:'iswr',0x68:'xtop',0x69:'xitop',0x6c:'xgkick',
    0x64:'mfp',0x73:'erleng',0x7b:'waitp',
}
_CONTROL={name for name in _LOWER_PRIMARY.values()
          if name in {'b','bal','jr','jalr','ibeq','ibne','ibltz','ibgtz','iblez','ibgez'}}
_DIRECT_BRANCH={'b','bal','ibeq','ibne','ibltz','ibgtz','iblez','ibgez'}

_UPPER_DIRECT={
    0x00:'addx',0x01:'addy',0x02:'addz',0x03:'addw',
    0x08:'maddx',0x09:'maddy',0x0a:'maddz',0x0b:'maddw',
    0x10:'maxx',0x11:'maxy',0x12:'maxz',0x13:'maxw',
    0x18:'mulx',0x19:'muly',0x1a:'mulz',0x1b:'mulw',
    0x1c:'mulq',0x1f:'minii',0x20:'addq',0x22:'addi',
    0x28:'add',0x29:'madd',0x2a:'mul',0x2c:'sub',
}
_UPPER_SPECIAL={
    0x00:'addax',0x01:'adday',0x02:'addaz',0x03:'addaw',
    0x08:'maddax',0x09:'madday',0x0a:'maddaz',0x0b:'maddaw',
    0x10:'itof0',0x11:'itof4',0x12:'itof12',0x13:'itof15',
    0x14:'ftoi0',0x15:'ftoi4',0x16:'ftoi12',0x17:'ftoi15',
    0x18:'mulax',0x19:'mulay',0x1a:'mulaz',0x1b:'mulaw',
    0x1f:'clip',0x2f:'nop',
}


def _special_opcode(word):
    # Sony VU User's Manual type-3 encoding: the low two bits and bits 6..10
    # form the extended operation number when the low six-bit opcode selects
    # one of the four SPECIAL encodings.
    return (word&3)|((word>>4)&0x7c)


def _lower_decode(lower,immediate):
    if immediate:
        return 'immediate',{'value':lower}
    primary=lower>>25
    if primary!=0x40:
        name=_LOWER_PRIMARY.get(primary,'other')
    else:
        low=lower&0x3f
        name=(_LOWER_SPECIAL.get(_special_opcode(lower),'other')
              if low>=0x3c else _LOWER_DIRECT.get(low,'other'))
        # The architectural lower NOP is the documented MOVE VF00,VF00 form.
        if name=='move' and lower==0x8000033c:name='nop'
    fields={'is':(lower>>11)&31,'it':(lower>>16)&31}
    if name in {'lq','sq'}:
        fields.update(dest=(lower>>21)&15,imm11=_signed(lower&0x7ff,11))
    elif name in {'isw','ilw'}:
        fields.update(dest=(lower>>21)&15,imm11=_signed(lower&0x7ff,11))
    elif name in {'iaddiu','isubiu'}:
        fields['imm15']=((lower>>10)&0x7800)|(lower&0x7ff)
    elif name in {'iadd','isub','iaddi','iand','ior'}:
        fields['id']=(lower>>6)&31
        if name=='iaddi':fields['imm5']=_signed((lower>>6)&31,5)
    elif name in {'fcset','fcand'}:
        fields={'imm24':lower&0xffffff}
    elif name in {'move','mr32','lqi','sqi','lqd','sqd'}:
        fields.update(dest=(lower>>21)&15,ft=(lower>>16)&31,fs=(lower>>11)&31)
    elif name in {'div','sqrt','rsqrt'}:
        fields.update(fsf=(lower>>21)&3,ftf=(lower>>23)&3,fs=(lower>>11)&31,ft=(lower>>16)&31)
    elif name=='mtir':
        fields={'fsf':(lower>>21)&3,'it':(lower>>16)&31,'fs':(lower>>11)&31}
    elif name=='mfir':
        fields={'dest':(lower>>21)&15,'ft':(lower>>16)&31,'is':(lower>>11)&31}
    elif name=='mfp':
        fields={'dest':(lower>>21)&15,'ft':(lower>>16)&31}
    elif name=='erleng':
        fields={'fs':(lower>>11)&31}
    elif name in {'xtop','xitop'}:
        fields={'it':(lower>>16)&31}
    elif name=='xgkick':
        fields={'is':(lower>>11)&31}
    elif name in _CONTROL:
        fields={'is':(lower>>11)&31,'it':(lower>>16)&31,'imm11':_signed(lower&0x7ff,11)}
    elif name in {'waitq','waitp','nop'}:
        fields={}
    return name,fields


def _upper_decode(upper):
    low=upper&0x3f
    special=low>=0x3c
    if special:name=_UPPER_SPECIAL.get(_special_opcode(upper),'other')
    else:name=_UPPER_DIRECT.get(low,'other')
    if special:
        fields={'dest':(upper>>21)&15,'ft':(upper>>16)&31,'fs':(upper>>11)&31}
    else:
        fields={'dest':(upper>>21)&15,'ft':(upper>>16)&31,'fs':(upper>>11)&31,'fd':(upper>>6)&31}
    if name=='nop':fields={}
    elif name=='clip':fields={'fs':(upper>>11)&31,'ft':(upper>>16)&31}
    return name,fields


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
    upper_name: str
    lower_fields: dict
    upper_fields: dict
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
    name,lower_fields=_lower_decode(lower,immediate)
    upper_name,upper_fields=_upper_decode(upper)
    target=None
    if name in _DIRECT_BRANCH:
        # The signed offset is measured in 64-bit instruction pairs. The
        # documented branch base is the pair following the delay pair (+8).
        target=address+8+_signed(lower&0x7ff,11)*8
        if target<0:
            raise ValueError('VU branch target precedes address zero')
    return VuPair(address,lower,upper,opcode,upper&0x3f,end,immediate,name,upper_name,
                  lower_fields,upper_fields,target)


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
            'control_pairs':[pair.report() for pair in pairs if pair.lower_name in _CONTROL],
            'direct_edges':edges,
            'missing_branch_delay_slots':sorted({edge['delay_slot'] for edge in edges if not edge['delay_slot_in_program']}),
            'external_direct_targets':sorted({edge['target'] for edge in edges if not edge['in_program']}),
            'unsupported_pairs':[{'address':pair.address,'lower':pair.lower_name,'upper':pair.upper_name}
                                 for pair in pairs if pair.lower_name=='other' or pair.upper_name=='other'],
            'upper_opcode_counts':[{'opcode':opcode,'count':count} for opcode,count in sorted(upper_counts.items())],
            'lower_opcode_counts':[{'opcode':opcode,'count':count} for opcode,count in sorted(lower_counts.items())],
            'note':'Structural VU pair/control-flow analysis only; no VU execution or interpreter fallback.'}
