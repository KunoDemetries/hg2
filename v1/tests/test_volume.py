import tempfile
from pathlib import Path
import unittest
from hgtool.volume import Volume,BLOCK


def dual(value,width):return value.to_bytes(width,'little')+value.to_bytes(width,'big')


def record(name,extent,size,directory=False):
    length=33+len(name)+(len(name)%2==0)
    r=bytearray(length);r[0]=length
    r[2:10]=dual(extent,4);r[10:18]=dual(size,4)
    r[25]=2 if directory else 0;r[28:32]=dual(1,2)
    r[32]=len(name);r[33:33+len(name)]=name
    return r


def fixture():
    b=bytearray(32*BLOCK);p=bytearray(BLOCK)
    p[:7]=b'\x01CD001\x01';p[40:72]=b'SYNTHETIC'.ljust(32)
    p[80:88]=dual(32,4);p[120:124]=dual(1,2);p[124:128]=dual(1,2)
    p[128:132]=dual(BLOCK,2);p[156:190]=record(b'\0',20,BLOCK,True)
    b[16*BLOCK:17*BLOCK]=p;b[17*BLOCK:17*BLOCK+7]=b'\xffCD001\x01'
    root=record(b'\0',20,BLOCK,True)+record(b'\1',20,BLOCK,True)+record(b'DIR',21,BLOCK,True)
    child=record(b'\0',21,BLOCK,True)+record(b'\1',20,BLOCK,True)+record(b'FILE.BIN;1',24,3000)
    b[20*BLOCK:20*BLOCK+len(root)]=root;b[21*BLOCK:21*BLOCK+len(child)]=child
    b[24*BLOCK:24*BLOCK+3000]=bytes(n%251 for n in range(3000))
    return b


class VolumeTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        self.path=Path(self.temp.name)/'synthetic-volume.bin'

    def test_shifted_volume_and_cross_sector_read(self):
        self.path.write_bytes(bytes(3*BLOCK)+fixture())
        v=Volume(self.path,3*BLOCK);index=v.index()
        self.assertEqual(set(index),{'/DIR','/DIR/FILE.BIN;1'})
        self.assertEqual(b''.join(v.chunks(index['/DIR/FILE.BIN;1'],2040,30)),bytes(n%251 for n in range(2040,2070)))
        with self.assertRaises(ValueError):list(v.chunks(index['/DIR/FILE.BIN;1'],2990,11))
        with self.assertRaises(ValueError):list(v.chunks(index['/DIR']))

    def test_inconsistent_endianness_and_truncation(self):
        b=fixture();b[16*BLOCK+84]^=1;self.path.write_bytes(b)
        with self.assertRaises(ValueError):Volume(self.path)
        self.path.write_bytes(fixture()[:-1])
        with self.assertRaises(ValueError):Volume(self.path)

    def test_cycle_and_unsupported_extent(self):
        b=fixture();r=record(b'LOOP',20,BLOCK,True)
        b[21*BLOCK:21*BLOCK+len(r)]=r;self.path.write_bytes(b)
        with self.assertRaises(ValueError):Volume(self.path).index()
        b=fixture();b[21*BLOCK+68+25]|=0x80;self.path.write_bytes(b)
        with self.assertRaises(ValueError):Volume(self.path).index()

    def test_extent_bounds_and_record_boundary(self):
        b=fixture();offset=21*BLOCK+68;b[offset+2:offset+10]=dual(31,4)
        self.path.write_bytes(b)
        with self.assertRaises(ValueError):Volume(self.path).index()
        b=fixture();b[20*BLOCK]=255;self.path.write_bytes(b)
        with self.assertRaises(ValueError):Volume(self.path).index()
