"""Independent, read-only inventory of local IOP ELF sections and import ABIs.

This does not relocate or execute an IRX and does not apply EE decoder semantics.
"""
from collections import Counter
import hashlib
from pathlib import Path
import struct


def inspect_irx(path):
    path=Path(path)
    if path.stat().st_size>16*1024*1024:raise ValueError('IRX inventory size limit exceeded')
    data=path.read_bytes()
    return inspect_irx_data(data,str(path))


def inspect_irx_data(data,label='<memory>'):
    if len(data)>16*1024*1024:raise ValueError('IRX inventory size limit exceeded')
    if len(data)<52:raise ValueError('truncated IRX ELF header')
    h=struct.unpack_from('<16sHHIIIIIHHHHHH',data)
    if h[0][:7]!=b'\x7fELF\x01\x01\x01' or h[1:4]!=(0xff80,8,1) or h[8]!=52:
        raise ValueError('expected little-endian IOP relocatable ELF type 0xff80')
    if h[11]!=40 or not 0<h[12]<=4096 or h[13]>=h[12] or h[6]+h[12]*40>len(data):
        raise ValueError('invalid IRX section table')
    segments=[]
    if h[10]:
        if h[9]!=32 or h[5]+h[10]*32>len(data):raise ValueError('invalid IRX program header table')
        for n in range(h[10]):
            kind,offset,address,_,file_size,memory_size,flags,alignment=struct.unpack_from('<8I',data,h[5]+n*32)
            if kind!=1:continue
            if file_size>memory_size or offset+file_size>len(data) or address+memory_size>2**32:
                raise ValueError('invalid IRX load segment')
            if any(address<s['address']+s['memory_size'] and s['address']<address+memory_size for s in segments):
                raise ValueError('overlapping IRX load segments')
            segments.append({'address':address,'offset':offset,'file_size':file_size,
                             'memory_size':memory_size,'flags':flags,'alignment':alignment})
    raw=[struct.unpack_from('<10I',data,h[6]+n*40) for n in range(h[12])]
    for s in raw:
        if s[1]!=8 and s[4]+s[5]>len(data):raise ValueError('IRX section outside file')
        if s[3]+s[5]>2**32:raise ValueError('IRX section address overflow')
    strings=raw[h[13]]
    if strings[1]!=3:raise ValueError('section names are not a string table')
    names=data[strings[4]:strings[4]+strings[5]]
    sections=[]
    for s in raw:
        if s[0]>=len(names) or b'\0' not in names[s[0]:]:raise ValueError('invalid IRX section name')
        name=names[s[0]:].split(b'\0',1)[0].decode('ascii')
        sections.append({'name':name,'type':s[1],'flags':s[2],'address':s[3],
                         'offset':s[4],'size':s[5]})
    relocations=[];relocated_words=set()
    for s in raw:
        if s[1]!=9:continue
        if s[9]!=8 or s[5]%8 or not 0<s[7]<len(raw):raise ValueError('invalid IRX relocation table')
        target=raw[s[7]]
        for location,info in struct.iter_unpack('<II',data[s[4]:s[4]+s[5]]):
            # The local IRXs encode virtual offsets, including nonzero .data base.
            if location%4 or not target[3]<=location or location+4>target[3]+target[5]:
                raise ValueError('IRX relocation is outside its target section')
            relocations.append({'address':location,'type':info&255,'symbol':info>>8})
            if info&255==2:relocated_words.add(location)
    imports=[];exports=[]
    for section in sections:
        if section['type']!=1 or not section['flags']&4:continue
        payload=data[section['offset']:section['offset']+section['size']]
        offset=0
        while offset+20<=len(payload):
            magic=struct.unpack_from('<I',payload,offset)[0]
            if magic not in (0x41e00000,0x41c00000):offset+=4;continue
            next_pointer,version,mode=struct.unpack_from('<IHH',payload,offset+4)
            name=payload[offset+12:offset+20].split(b'\0',1)[0]
            if next_pointer or mode or not name or any(not (32<c<127) for c in name):
                raise ValueError('unsupported or invalid unlinked IRX library table')
            table={'name':name.decode('ascii'),'version':version,'address':section['address']+offset}
            cursor=offset+20;items=[]
            if magic==0x41e00000:
                while cursor+8<=len(payload):
                    jump,ordinal=struct.unpack_from('<II',payload,cursor)
                    if jump==ordinal==0:cursor+=8;break
                    if jump!=0x03e00008 or ordinal&0xffff0000!=0x24000000:
                        raise ValueError('invalid unlinked IRX import stub')
                    items.append({'ordinal':ordinal&65535,'stub':section['address']+cursor})
                    cursor+=8
                else:raise ValueError('unterminated IRX import table')
                table['imports']=items;imports.append(table)
            else:
                while cursor+4<=len(payload):
                    pointer=struct.unpack_from('<I',payload,cursor)[0]
                    if pointer==0 and section['address']+cursor not in relocated_words:cursor+=4;break
                    if pointer%4:raise ValueError('unaligned IRX export pointer')
                    items.append({'ordinal':len(items),'target':pointer});cursor+=4
                else:raise ValueError('unterminated IRX export table')
                table['exports']=items;exports.append(table)
            offset=cursor
    return {'path':label,'sha256':hashlib.sha256(data).hexdigest(),'entry':h[4],
            'sections':sections,'segments':segments,'imports':imports,'exports':exports,
            'relocation_counts':dict(sorted(Counter(r['type'] for r in relocations).items())),
            'relocations':relocations}
