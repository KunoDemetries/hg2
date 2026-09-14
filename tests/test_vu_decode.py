import unittest

from hgtool.vu_decode import decode_pair, decode_program, report_program


class VuDecodeTests(unittest.TestCase):
    def test_decodes_little_endian_pair_flags_and_direct_branch(self):
        # B with an offset of -1: its documented +8 branch base targets itself.
        pair=decode_pair(0x40,(0xc0000000<<32)|(0x20<<25)|0x7ff)
        self.assertEqual(pair.lower,0x400007ff)
        self.assertEqual(pair.upper,0xc0000000)
        self.assertEqual(pair.lower_opcode,0x20)
        self.assertEqual(pair.upper_opcode,0)
        self.assertTrue(pair.end)
        self.assertTrue(pair.lower_is_immediate)
        self.assertEqual(pair.lower_name,'immediate')
        self.assertIsNone(pair.branch_target)
        pair=decode_pair(0x40,(0x40000000<<32)|(0x20<<25)|0x7ff)
        self.assertEqual(pair.lower_name,'b')
        self.assertEqual(pair.branch_target,0x40)

    def test_reports_control_pairs_without_executing_them(self):
        data=((0x20<<25)|1).to_bytes(4,'little')+(0).to_bytes(4,'little')
        data+=(0).to_bytes(8,'little')
        report=report_program(data,0x80)
        self.assertEqual(report['pair_count'],2)
        self.assertEqual(report['control_pairs'][0]['branch_target'],0x90)
        self.assertEqual(report['direct_edges'],[{'source':0x80,'delay_slot':0x88,'target':0x90,'kind':'b',
                                                  'delay_slot_in_program':True,'in_program':False}])
        self.assertEqual(report['missing_branch_delay_slots'],[])
        self.assertEqual(report['external_direct_targets'],[0x90])
        self.assertEqual(report['upper_opcode_counts'],[{'opcode':0,'count':2}])
        self.assertEqual(report['lower_opcode_counts'],[{'opcode':0,'count':1},{'opcode':0x20,'count':1}])
        self.assertEqual(report['end_pairs'],[])

    def test_rejects_malformed_program_and_address(self):
        with self.assertRaises(ValueError): decode_program(b'\0'*7)
        with self.assertRaises(ValueError): decode_pair(1,0)

    def test_reports_a_missing_branch_delay_slot_as_an_aot_boundary(self):
        data=(0x20<<25).to_bytes(4,'little')+(0).to_bytes(4,'little')
        report=report_program(data,0)
        self.assertEqual(report['missing_branch_delay_slots'],[8])
        self.assertFalse(report['direct_edges'][0]['delay_slot_in_program'])
