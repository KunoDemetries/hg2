"""Read-only ROMDIR container subset established from the supplied IOPRP image.

This accepts reboot images with an empty RESET member and a directory at offset
zero. It is not a BIOS loader and does not execute any supplied bytes.
"""
from pathlib import Path
import hashlib
import struct


class RebootImage:
    def __init__(self,path):
        self.path=Path(path)
        if self.path.stat().st_size>64*1024*1024:raise ValueError('reboot image size limit exceeded')
        self.data=self.path.read_bytes();self.sha256=hashlib.sha256(self.data).hexdigest()
        if len(self.data)<48:raise ValueError('truncated reboot directory')
        name,_,size=struct.unpack_from('<10sHI',self.data)
        if name!=b'RESET\0\0\0\0\0' or size:raise ValueError('only empty-RESET reboot images supported')
        name,_,directory_size=struct.unpack_from('<10sHI',self.data,16)
        if name!=b'ROMDIR\0\0\0\0' or directory_size%16 or not 48<=directory_size<=65536 or directory_size>len(self.data):
            raise ValueError('invalid reboot directory size/name')
        self.entries={};offset=0;extended_total=0
        for position in range(0,directory_size,16):
            raw,extended,size=struct.unpack_from('<10sHI',self.data,position)
            if raw==bytes(10):
                if extended or size or any(self.data[position:directory_size]):raise ValueError('nonzero directory terminator/padding')
                break
            name=raw.split(b'\0',1)[0]
            if not name or any(raw[len(name):]) or any(not (c==95 or 48<=c<=57 or 65<=c<=90) for c in name):
                raise ValueError('invalid reboot member name')
            name=name.decode('ascii')
            if name in self.entries or offset+size>len(self.data):raise ValueError('duplicate or out-of-bounds reboot member')
            self.entries[name]={'name':name,'offset':offset,'size':size,'extended_size':extended,
                                'extended_offset':extended_total}
            extended_total+=extended;offset+=(size+15)&~15
        else:raise ValueError('reboot directory has no terminator')
        if 'EXTINFO' not in self.entries or self.entries['EXTINFO']['size']!=extended_total:
            raise ValueError('extended-info size does not match member metadata')

    def read(self,name):
        entry=self.entries[name]
        return self.data[entry['offset']:entry['offset']+entry['size']]

    def inventory(self):
        from .irx import inspect_irx_data
        modules=[]
        for name in self.entries:
            data=self.read(name)
            if data.startswith(b'\x7fELF'):
                modules.append(inspect_irx_data(data,f'{self.path}#{name}'))
        return {'path':str(self.path),'sha256':self.sha256,'entries':list(self.entries.values()),'modules':modules}
