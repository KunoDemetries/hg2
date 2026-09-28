import unittest

from hgtool.triage import make_triage


class Instruction:
    def __init__(self,name,word,rs=0,rt=0): self.name=name; self.word=word; self.rs=rs; self.rt=rt


class TriageTests(unittest.TestCase):
    def test_groups_and_sorts_boundaries_without_changing_them(self):
        report={
            'instruction_count': 9,
            'complete': False,
            'issues': [
                {'pc':0x30,'reason':'unsupported','word':0x1234},
                {'pc':0x10,'reason':'unresolved indirect transfer','register':5},
                {'pc':0x20,'reason':'unsupported','word':0x5678},
            ],
        }
        triage=make_triage({0x8:Instruction('lw',0x8c990000),0xc:Instruction('nop',0),
                            0x10:Instruction('jr',0x00a00008),0x20:Instruction('unsupported',0x5678)},report)
        self.assertEqual(triage['unresolved_count'],3)
        self.assertEqual([(group['reason'],group['count']) for group in triage['groups']],
                         [('unsupported',2),('unresolved indirect transfer',1)])
        self.assertEqual(triage['groups'][0]['sites'][0]['pc'],0x20)
        self.assertEqual(triage['groups'][1]['sites'][0]['register'],5)
        self.assertEqual(triage['groups'][1]['sites'][0]['word'],0x00a00008)
        self.assertEqual(triage['unresolved_mnemonics'],{'jr':1,'unsupported':1})
        self.assertEqual(triage['unresolved_shapes'],[
            {'reason':'unresolved indirect transfer','mnemonic':'jr','register':5,'count':1},
            {'reason':'unsupported','mnemonic':None,'count':1},
            {'reason':'unsupported','mnemonic':'unsupported','count':1},
        ])
        self.assertEqual(triage['indirect_setup_shapes'],[
            {'mnemonic':'jr','register':5,'previous':['lw','nop'],'count':1},
        ])
        self.assertEqual(triage['indirect_load_shapes'],[
            {'mnemonic':'jr','register':5,'setup':'other','count':1,'examples':[0x10]},
        ])
        self.assertEqual(triage['file_pointer_candidates'],[])
