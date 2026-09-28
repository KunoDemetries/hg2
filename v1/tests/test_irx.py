import struct
import tempfile
from pathlib import Path
import unittest
from hgtool.irx import inspect_irx


def fixture():
    b=bytearray(0x500)
    b[:52]=struct.pack('<16sHHIIIIIHHHHHH',b'\x7fELF\x01\x01\x01'+bytes(9),
                      0xff80,8,1,0,0,0x400,0,52,0,0,40,4,1)
    names=b'\0.shstrtab\0.text\0.rel.text\0';b[0x300:0x300+len(names)]=names
    text=struct.pack('<IIHH8s',0x41e00000,0,0x102,0,b'synthetic')
    text+=struct.pack('<4I',0x03e00008,0x24000007,0,0)
    text+=struct.pack('<IIHH8s',0x41c00000,0,0x101,0,b'exports')
    text+=struct.pack('<3I',0,8,0) # Relocated target zero is an export, not terminator.
    b[0x100:0x100+len(text)]=text
    b[0x200:0x208]=struct.pack('<II',56,2)
    sections=[(0,)*10,(1,3,0,0,0x300,len(names),0,0,1,0),
              (11,1,6,0,0x100,len(text),0,0,4,0),(17,9,0,0,0x200,8,0,2,4,8)]
    for n,s in enumerate(sections):struct.pack_into('<10I',b,0x400+n*40,*s)
    return b


class IrxTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        self.path=Path(self.temp.name)/'synthetic.irx'

    def test_imports_exports_and_zero_relocation(self):
        self.path.write_bytes(fixture());r=inspect_irx(self.path)
        self.assertEqual(r['imports'][0]['imports'],[{'ordinal':7,'stub':20}])
        self.assertEqual(r['exports'][0]['exports'],[{'ordinal':0,'target':0},{'ordinal':1,'target':8}])
        self.assertEqual(r['relocation_counts'],{2:1})

    def test_invalid_stub_and_relocation_bounds(self):
        b=fixture();b[0x100+27]=0xff;self.path.write_bytes(b)
        with self.assertRaises(ValueError):inspect_irx(self.path)
        b=fixture();struct.pack_into('<I',b,0x200,0x1000);self.path.write_bytes(b)
        with self.assertRaises(ValueError):inspect_irx(self.path)

    def test_section_bounds_and_wrong_architecture(self):
        b=fixture();struct.pack_into('<I',b,0x400+80+20,0xffffffff);self.path.write_bytes(b)
        with self.assertRaises(ValueError):inspect_irx(self.path)
        b=fixture();b[18]=62;self.path.write_bytes(b)
        with self.assertRaises(ValueError):inspect_irx(self.path)
