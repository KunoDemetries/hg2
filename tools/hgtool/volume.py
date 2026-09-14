"""Read-only ECMA-119 subset, with an explicit volume origin for the local CVM.

No extraction or CRI implementation is used. Unsupported filesystem features fail.
"""
from dataclasses import dataclass
from pathlib import Path
import hashlib

BLOCK = 2048


def both(data, offset, width):
    field=data[offset:offset+2*width]
    if len(field)!=2*width:raise ValueError('truncated both-endian integer')
    little=int.from_bytes(field[:width],'little')
    if little!=int.from_bytes(field[width:],'big'):
        raise ValueError('inconsistent both-endian integer')
    return little


@dataclass(frozen=True)
class Entry:
    path: str
    extent: int
    size: int
    directory: bool


class Volume:
    def __init__(self,path,origin=0):
        self.path=Path(path)
        self.origin=origin
        self.file_size=self.path.stat().st_size
        if type(origin) is not int or origin<0 or origin%BLOCK:
            raise ValueError('volume origin must be a nonnegative sector-aligned offset')
        primary=None
        with self.path.open('rb') as f:
            for sector in range(16,80):
                d=self._read(f,sector*BLOCK,BLOCK)
                if d[1:6]!=b'CD001' or d[6]!=1:raise ValueError('invalid volume descriptor')
                if d[0]==1:
                    if primary is not None:raise ValueError('multiple primary descriptors unsupported')
                    primary=d
                if d[0]==255:break
            else:raise ValueError('volume descriptor terminator missing')
        if primary is None:raise ValueError('primary volume descriptor missing')
        if both(primary,128,2)!=BLOCK:raise ValueError('only 2048-byte logical blocks supported')
        if both(primary,120,2)!=1 or both(primary,124,2)!=1:
            raise ValueError('multiple-volume sets unsupported')
        self.blocks=both(primary,80,4)
        if self.origin+self.blocks*BLOCK>self.file_size:
            raise ValueError('declared volume extends past container')
        self.label=primary[40:72].decode('ascii').rstrip()
        self.root=self._record(primary[156:190],'')
        if not self.root.directory:raise ValueError('root record is not a directory')

    def _read(self,f,offset,size):
        if offset<0 or size<0 or self.origin+offset+size>self.file_size:
            raise ValueError('volume read outside container')
        f.seek(self.origin+offset)
        data=f.read(size)
        if len(data)!=size:raise ValueError('short volume read')
        return data

    def _record(self,r,path):
        if len(r)<34 or r[0]!=len(r) or len(r)%2 or not r[32] or 33+r[32]>len(r):
            raise ValueError('malformed directory record')
        if r[1] or r[26] or r[27] or (r[25]&~3):
            raise ValueError('extended attributes, interleaving or advanced file flags unsupported')
        if both(r,28,2)!=1:raise ValueError('directory references another volume')
        extent,size=both(r,2,4),both(r,10,4)
        if extent*BLOCK+size>self.blocks*BLOCK:
            raise ValueError('file extent outside volume')
        return Entry(path,extent,size,bool(r[25]&2))

    def index(self):
        entries={}
        pending=[self.root];visited=set()
        with self.path.open('rb') as f:
            while pending:
                directory=pending.pop()
                if directory.extent in visited:raise ValueError('directory cycle or alias')
                visited.add(directory.extent)
                if len(visited)>4096 or directory.size>16*1024*1024:
                    raise ValueError('directory traversal limit exceeded')
                data=self._read(f,directory.extent*BLOCK,directory.size)
                offset=0
                while offset<len(data):
                    length=data[offset]
                    if not length:
                        end=min(len(data),(offset//BLOCK+1)*BLOCK)
                        if any(data[offset:end]):raise ValueError('nonzero directory sector padding')
                        offset=end;continue
                    if offset%BLOCK+length>BLOCK or offset+length>len(data):
                        raise ValueError('directory record crosses sector or extent')
                    r=data[offset:offset+length];offset+=length
                    item=self._record(r,'')
                    name=r[33:33+r[32]]
                    if name in (b'\0',b'\1'):
                        if not item.directory:raise ValueError('special directory identifier on a file')
                        continue
                    name=name.decode('ascii')
                    if any(ord(c)<32 or ord(c)>126 for c in name) or any(c in name for c in '/\\:') or name in ('.','..'):
                        raise ValueError('invalid volume filename')
                    path=directory.path+'/'+name
                    if path in entries:raise ValueError('duplicate volume path')
                    item=Entry(path,item.extent,item.size,item.directory)
                    entries[path]=item
                    if len(entries)>200000:raise ValueError('volume entry limit exceeded')
                    if item.directory:pending.append(item)
        return entries

    def chunks(self,entry,offset=0,size=None):
        if entry.directory:raise ValueError('cannot read directory as asset')
        size=entry.size-offset if size is None else size
        if offset<0 or size<0 or offset+size>entry.size:
            raise ValueError('asset read outside file')
        with self.path.open('rb') as f:
            while size:
                count=min(size,1024*1024)
                yield self._read(f,entry.extent*BLOCK+offset,count)
                offset+=count;size-=count

    def digest(self,entry):
        h=hashlib.sha256()
        for chunk in self.chunks(entry):h.update(chunk)
        return h.hexdigest()
