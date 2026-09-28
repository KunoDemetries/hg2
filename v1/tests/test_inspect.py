import unittest
from hgtool.decode import decode
from hgtool.inspect import access


class InspectTests(unittest.TestCase):
    def test_signed_displacement_and_wrap(self):
        regs=[0]*32;regs[2]=0x10010000
        self.assertEqual(access(decode(0,(35<<26)|(2<<21)|0xe010),regs),(0x1000e010,4))
        regs[2]=0
        self.assertEqual(access(decode(0,(43<<26)|(2<<21)|0xfffc),regs),(0xfffffffc,4))

    def test_quad_alignment_and_nonmemory(self):
        regs=[0]*32;regs[2]=0x1003
        self.assertEqual(access(decode(0,(30<<26)|(2<<21)|4),regs),(0x1000,16))
        self.assertIsNone(access(decode(0,0),regs))
