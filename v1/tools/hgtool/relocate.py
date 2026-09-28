"""Build-time MIPS relocation effects for the symbol-zero local IRX subset.

Input words are immutable; unsupported records and malformed pairs fail before
any result is returned. This is not a runtime linker or instruction interpreter.
"""


def signed16(value):
    value&=65535
    return value-65536 if value&32768 else value


def relocate_words(words,records,base):
    if type(base) is not int or not 0<=base<2**32 or base%4:
        raise ValueError('relocation base must be an aligned 32-bit address')
    result=dict(words);seen=set();index=0
    def word(record):
        address=record['address']
        if record['symbol']!=0:raise ValueError('symbolic IRX relocation is unsupported')
        if address in seen or address%4 or address not in words:
            raise ValueError('duplicate, unaligned or missing relocation word')
        seen.add(address)
        return words[address]
    while index<len(records):
        r=records[index];old=word(r);location=r['address'];kind=r['type']
        if kind==2:result[location]=(old+base)&0xffffffff
        elif kind==4:
            if old>>26 not in (2,3):raise ValueError('R_MIPS_26 is not a jump instruction')
            target=((old&0x03ffffff)<<2)|(location&0xf0000000)
            target+=base
            if target>=2**32 or (target&0xf0000000)!=((location+base+4)&0xf0000000):
                raise ValueError('relocated jump crosses its 256 MiB region')
            result[location]=(old&0xfc000000)|((target>>2)&0x03ffffff)
        elif kind==5:
            if index+1==len(records) or records[index+1]['type']!=6:
                raise ValueError('HI16 requires the following LO16 relocation')
            low_record=records[index+1];low=word(low_record)
            full=((old&65535)<<16)+signed16(low)+base
            result[location]=(old&0xffff0000)|(((full+0x8000)>>16)&65535)
            result[low_record['address']]=(low&0xffff0000)|(full&65535)
            index+=1
        else:raise ValueError(f'unsupported IRX relocation type {kind}')
        index+=1
    return result


def relocate_sections(data,inventory,base):
    """Return relocated file-backed allocatable sections for future AOT lowering."""
    import hashlib
    import struct
    if hashlib.sha256(data).hexdigest()!=inventory['sha256']:
        raise ValueError('IRX relocation identity mismatch')
    words={};sections=[]
    for section in inventory['sections']:
        if section['type']!=1 or not section['flags']&2 or not section['size']:continue
        a,size=section['address'],section['size']
        if a%4 or size%4 or a+size+base>2**32:
            raise ValueError('unaligned or overflowing relocatable section')
        raw=data[section['offset']:section['offset']+size]
        for n,(value,) in enumerate(struct.iter_unpack('<I',raw)):
            if a+n*4 in words:raise ValueError('overlapping IRX allocatable sections')
            words[a+n*4]=value
        sections.append(section)
    relocated=relocate_words(words,inventory['relocations'],base)
    return [{'name':s['name'],'address':s['address']+base,
             'data':b''.join(struct.pack('<I',relocated[a]) for a in range(s['address'],s['address']+s['size'],4))}
            for s in sections]
