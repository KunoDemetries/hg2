"""Read-only diagnostic views using this project's decoder and native snapshots."""
import json
from pathlib import Path
import struct
from .decode import decode


def access(i,regs):
    sizes={'lb':1,'lbu':1,'sb':1,'lh':2,'lhu':2,'sh':2,'lw':4,'lwu':4,
           'sw':4,'lwc1':4,'swc1':4,'ld':8,'sd':8,'lq':16,'sq':16}
    if i.name not in sizes:return None
    address=(regs[i.rs]+i.immediate)&0xffffffff
    if i.name in ('lq','sq'):address&=~15
    return address,sizes[i.name]


def listing(image, start, count):
    if type(start) is not int or start%4 or not 0<=start<2**32 or not 1<=count<=1024:
        raise ValueError('inspection needs an aligned address and count from 1 to 1024')
    for pc in range(start,min(start+count*4,2**32),4):
        if not image.executable(pc):
            print(f'{pc:08x} outside configured executable bytes')
            break
        i=decode(pc,image.word(pc))
        target=f' target=0x{i.target:08x}' if i.target is not None else ''
        print(f'{pc:08x} {i.word:08x} {i.name:12} rs={i.rs} rt={i.rt} rd={i.rd} sa={i.sa} imm={i.immediate}{target}')


def snapshot(image, prefix):
    meta=json.loads(Path(str(prefix)+'.json').read_text())
    if meta['image_sha256']!=image.sha256:
        raise ValueError('snapshot executable identity mismatch')
    ram_path=Path(str(prefix)+'.ram')
    if ram_path.stat().st_size!=32*1024*1024:
        raise ValueError('snapshot must contain exactly 32 MiB of guest RAM')
    ram=ram_path.read_bytes()
    raw_regs=meta['gpr']
    if not isinstance(raw_regs,list) or len(raw_regs)!=32 or any(
        not isinstance(v,list) or len(v)!=2 or any(not isinstance(h,str) for h in v)
        for v in raw_regs):raise ValueError('snapshot must contain 32 pairs of register strings')
    pairs=[[int(h,0) for h in v] for v in raw_regs]
    if any(not 0<=h<2**64 for v in pairs for h in v):raise ValueError('snapshot register outside 64-bit range')
    regs=[v[0] for v in pairs];regs[0]=0
    print('Stopped instructions:')
    listing(image,meta['pc'],16)
    if image.executable(meta['pc']):
        operation=access(decode(meta['pc'],image.word(meta['pc'])),regs)
        if operation:
            address,size=operation
            print(f'Stopped memory access: address=0x{address:08x}, width={size} bytes')
    transfer=meta.get('last_transfer_pc',0)
    if transfer>=16 and image.executable(transfer):
        print('Last control transfer:')
        listing(image,transfer-16,6)
    ra=regs[31]&0xffffffff
    if ra>=32 and image.executable(ra-32):
        print('Caller window (RA minus 32):')
        listing(image,ra-32,10)
    a0=regs[4]&0xffffffff
    if 0x80000000<=a0<0xc0000000: a0&=0x1fffffff
    if a0%4==0 and a0+4<=len(ram):
        pointer=struct.unpack_from('<I',ram,a0)[0]
        print(f'RAM[a0=0x{a0:08x}]=0x{pointer:08x}; possible object table, not an inferred type')
        if pointer%4==0:
            for offset in range(0,64,4):
                try: word=image.word(pointer+offset)
                except ValueError: break
                print(f'  +{offset:02x} [{pointer+offset:08x}] {word:08x}')
