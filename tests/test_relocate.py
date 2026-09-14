import unittest
from hgtool.relocate import relocate_words


def rel(a,t,s=0):return {'address':a,'type':t,'symbol':s}


class RelocationTests(unittest.TestCase):
    def test_word_jump_and_signed_low_carry(self):
        words={0:0xfffffff0,4:0x0c000010,8:0x3c040001,12:0x24848000}
        out=relocate_words(words,[rel(0,2),rel(4,4),rel(8,5),rel(12,6)],0x8000)
        self.assertEqual(out[0],0x7ff0)
        self.assertEqual(out[4],0x0c002010)
        self.assertEqual(out[8],0x3c040001)
        self.assertEqual(out[12],0x24840000)
        self.assertEqual(words[12],0x24848000)

    def test_positive_low_carry_to_adjusted_high(self):
        out=relocate_words({0:0x3c050001,4:0x24a57ff0},[rel(0,5),rel(4,6)],0x30)
        self.assertEqual(out,{0:0x3c050002,4:0x24a58020})

    def test_reject_malformed_and_unsupported_without_mutation(self):
        words={0:0x3c040001,4:0x24840000}
        for records in ([rel(0,5)],[rel(0,6)],[rel(0,2,1)],[rel(0,2),rel(0,2)],[rel(8,2)]):
            with self.assertRaises(ValueError):relocate_words(words,records,0x1000)
        self.assertEqual(words,{0:0x3c040001,4:0x24840000})

    def test_jump_region_overflow(self):
        with self.assertRaises(ValueError):
            relocate_words({0:0x0bffffff},[rel(0,4)],0x1000)
