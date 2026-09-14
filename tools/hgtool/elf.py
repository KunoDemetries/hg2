from dataclasses import dataclass
import hashlib
from pathlib import Path
import struct


@dataclass(frozen=True)
class Segment:
    offset: int
    address: int
    file_size: int
    memory_size: int
    flags: int


class Elf:
    def __init__(self, data: bytes):
        self.data = data
        self.overlays = []
        self.sha256 = hashlib.sha256(data).hexdigest()
        if len(data) < 52:
            raise ValueError("truncated ELF header")
        h = struct.unpack_from('<16sHHIIIIIHHHHHH', data)
        if h[0][:7] != b'\x7fELF\x01\x01\x01' or h[1:4] != (2, 8, 1):
            raise ValueError("expected little-endian ELF32 MIPS executable")
        self.entry = h[4]
        if h[8] != 52 or h[9] != 32 or h[5] + h[10] * 32 > len(data):
            raise ValueError("invalid ELF program header table")
        self.segments = []
        for i in range(h[10]):
            kind, off, va, _, fs, ms, flags, _ = struct.unpack_from('<8I', data, h[5] + i * 32)
            if kind != 1:
                continue
            if fs > ms or off + fs > len(data) or va + ms > 2**32:
                raise ValueError("invalid load segment")
            s = Segment(off, va, fs, ms, flags)
            if ms and any(va < t.address + t.memory_size and t.address < va + ms
                          for t in self.segments):
                raise ValueError("overlapping load segments")
            self.segments.append(s)
        if not self.executable(self.entry):
            raise ValueError("entry is not file-backed executable memory")

    @classmethod
    def read(cls, path):
        return cls(Path(path).read_bytes())

    def executable(self, address):
        return address % 4 == 0 and (any(o['address'] <= address and address+4 <= o['address']+o['size'] for o in self.overlays) or any(s.flags & 1 and s.address <= address
                                      and address + 4 <= s.address + s.file_size
                                      for s in self.segments))

    def add_overlay(self, overlay):
        source, dest, size = overlay['source'], overlay['address'], overlay['size']
        if not any(s.address<=source and source+size<=s.address+s.file_size for s in self.segments):
            raise ValueError('overlay source must be wholly file-backed in one ELF segment')
        physical = dest & 0x1fffffff if 0x80000000<=dest<0xc0000000 else dest
        if physical+size>32*1024*1024:
            raise ValueError('overlay destination must be direct RAM or its kernel alias')
        if any(physical<s.address+s.memory_size and s.address<physical+size for s in self.segments):
            raise ValueError('overlay overlaps original image memory')
        for o in self.overlays:
            other=o['address'] & 0x1fffffff if 0x80000000<=o['address']<0xc0000000 else o['address']
            if physical<other+o['size'] and other<physical+size:
                raise ValueError('overlapping overlay destinations are not supported yet')
        self.overlays.append(overlay)

    def word(self, address):
        if not self.executable(address):
            raise ValueError(f"not an executable word: 0x{address:08x}")
        for o in self.overlays:
            if o['address']<=address<o['address']+o['size']:
                address=o['source']+address-o['address']
                break
        s = next(s for s in self.segments if s.address <= address < s.address + s.file_size)
        return struct.unpack_from('<I', self.data, s.offset + address - s.address)[0]

    def file_word(self, address):
        """Read an aligned, initialized ELF word without implying it is code.

        This deliberately excludes BSS and overlays: callers can report an
        input-image pointer as diagnostic evidence, but cannot mistake it for
        a runtime-memory read or an executable transfer.
        """
        if address % 4:
            raise ValueError(f"unaligned file-backed word: 0x{address:08x}")
        for s in self.segments:
            if s.address <= address and address + 4 <= s.address + s.file_size:
                return struct.unpack_from('<I', self.data, s.offset + address - s.address)[0]
        raise ValueError(f"not a file-backed word: 0x{address:08x}")
