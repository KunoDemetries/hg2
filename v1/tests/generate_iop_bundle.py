"""Generate standalone, ROMDIR and embedded AOT loader/execution fixtures."""
import hashlib
from pathlib import Path
import struct
import sys
from test_irx import fixture
from hgtool.iop_build import bundle

output=Path(sys.argv[1]);root=Path(sys.argv[2]);root.mkdir(parents=True,exist_ok=True)
config='schema_version=1\nram_size=0x200000\nreserved_low=0xffd0\nreserved_high=0x1f0000\n'
for name,base,value in (('a',0x10000,11),('b',0x20000,22),('c',0x30000,33)):
    data=fixture()
    struct.pack_into('<I',data,28,52);struct.pack_into('<HH',data,42,32,1)
    struct.pack_into('<8I',data,52,1,0x100,0,0,68,0x100,7,16)
    struct.pack_into('<4I',data,0x100,0x03e00008,0x24020000|value,0x03e00008,0)
    digest=hashlib.sha256(data).hexdigest()
    config+=f'\n[[modules]]\nname="{name}"\nbase={base}\nsha256="{digest}"\nfunctions=[]\n'
    if name=='a':
        (root/'a.irx').write_bytes(data);config+='path="a.irx"\n'
    elif name=='b':
        container=b''.join(struct.pack('<10sHI',n,0,size) for n,size in (
            (b'RESET',0),(b'ROMDIR',80),(b'EXTINFO',0),(b'MODULE',len(data)),(b'',0)))+data
        (root/'b.img').write_bytes(container)
        config+=f'path="b.img"\nmember="MODULE"\ncontainer_sha256="{hashlib.sha256(container).hexdigest()}"\n'
    else:
        container=b'prefix!!'+data+b'trailer'
        (root/'c.img').write_bytes(container)
        config+=f'path="c.img"\noffset=8\nsize={len(data)}\ncontainer_sha256="{hashlib.sha256(container).hexdigest()}"\n'
path=root/'config.toml';path.write_text(config)
cpp,_=bundle(path,['a','b','c']);output.write_text(cpp,encoding='utf-8')
