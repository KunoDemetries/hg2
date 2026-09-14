import tempfile
import struct
from pathlib import Path
import unittest
from test_irx import fixture
from hgtool.romdir import RebootImage


def reboot():
    module=fixture()
    return b''.join(struct.pack('<10sHI',name,0,size) for name,size in (
        (b'RESET',0),(b'ROMDIR',80),(b'EXTINFO',0),(b'MODULE',len(module)),(b'',0)))+module


class RebootTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        self.path=Path(self.temp.name)/'reboot.img'

    def test_metadata_without_extraction(self):
        self.path.write_bytes(reboot());r=RebootImage(self.path)
        self.assertEqual(r.read('MODULE'),fixture())
        self.assertEqual(len(r.inventory()['modules']),1)
        self.assertEqual(r.entries['MODULE']['offset'],80)

    def test_reject_extent_and_extended_metadata_mismatch(self):
        self.path.write_bytes(reboot()[:-1])
        with self.assertRaises(ValueError):RebootImage(self.path)
        b=bytearray(reboot());struct.pack_into('<H',b,10,8);self.path.write_bytes(b)
        with self.assertRaises(ValueError):RebootImage(self.path)

    def test_duplicate_and_nonempty_reset(self):
        b=bytearray(reboot());b[48:58]=b'ROMDIR\0\0\0\0';self.path.write_bytes(b)
        with self.assertRaises(ValueError):RebootImage(self.path)
        b=bytearray(reboot());struct.pack_into('<I',b,12,16);self.path.write_bytes(b)
        with self.assertRaises(ValueError):RebootImage(self.path)
