import unittest
from hgtool.iop_build import discover
from hgtool.iop_decode import decode


class IopBuildTests(unittest.TestCase):
    def test_major_scan_summary_keeps_every_boundary(self):
        from hgtool.major_scan import summarize
        code={0:decode(0,0),4:decode(4,0x0000000c)}
        report={'issues':[{'pc':4,'reason':'syscall'}]}
        result=summarize(code,report,[{'module':'alpha','reachable_words':3,
                                       'issues':[{'reason':'break'}],
                                       'opcode_counts':[{'name':'addu','count':2},{'name':'break','count':1}]}])
        self.assertEqual(result['ee']['boundary_count'],1)
        self.assertEqual(result['iop']['boundary_reasons'],[{'name':'break','count':1}])
        self.assertEqual(result['iop']['boundary_families'],[{'name':'break','count':1}])
        self.assertEqual(result['iop']['opcode_counts'],[{'name':'addu','count':2},{'name':'break','count':1}])

    def test_residual_shapes_keep_all_boundaries_without_inferring_targets(self):
        from hgtool.iop_build import summarize_residual_issues
        code={0x1000:decode(0x1000,0x0080f809),0x2000:decode(0x2000,0x0000000c),
              0x3000:decode(0x3000,(0x12345<<6)|13)}
        issues=[{'module':'alpha','pc':0x1000,'reason':'unresolved indirect transfer','register':4},
                {'module':'beta','pc':0x1000,'reason':'unresolved indirect transfer','register':4},
                {'module':'alpha','pc':0x2000,'reason':'syscall'},
                {'module':'alpha','pc':0x3000,'reason':'break','code':0x12345}]
        self.assertEqual(summarize_residual_issues(code,issues),[
            {'reason':'unresolved indirect transfer','mnemonic':'jalr','register':4,'count':2,
             'examples':[{'module':'alpha','pc':0x1000},{'module':'beta','pc':0x1000}]},
            {'reason':'break','mnemonic':'break','code':0x12345,'count':1,
             'examples':[{'module':'alpha','pc':0x3000}]},
            {'reason':'syscall','mnemonic':'syscall','count':1,
             'examples':[{'module':'alpha','pc':0x2000}]},
        ])

    def test_indirect_setup_groups_preceding_load_shapes(self):
        from hgtool.iop_build import summarize_indirect_setups
        code={0x1000:decode(0x1000,0x8c850010), # lw a1,16(a0)
              0x1004:decode(0x1004,0x8ca20004), # lw v0,4(a1)
              0x1008:decode(0x1008,0x0040f809)} # jalr v0
        issues=[{'module':'alpha','pc':0x1008,'reason':'unresolved indirect transfer','register':2}]
        self.assertEqual(summarize_indirect_setups(code,issues),[{
            'mnemonic':'jalr','register':2,'previous':['lw','lw'],'setup':'loaded-base','count':1,
            'examples':[{'module':'alpha','pc':0x1008}]}])
        delayed={0x2000:decode(0x2000,0x8c850010),0x2004:decode(0x2004,0x8ca20004),
                 0x2008:decode(0x2008,0),0x200c:decode(0x200c,0x0040f809)}
        delayed_issue=[{'module':'beta','pc':0x200c,'reason':'unresolved indirect transfer','register':2}]
        self.assertEqual(summarize_indirect_setups(delayed,delayed_issue)[0]['setup'],'loaded-base')
        self.assertEqual(summarize_indirect_setups(delayed,delayed_issue)[0]['previous'],['lw','nop'])

    def test_import_stub_is_never_empty_success(self):
        words={0x1000:0x0c000800,0x1004:0,0x1008:0x03e00008,0x100c:0,
               0x2000:0x03e00008,0x2004:0x24000011}
        code,issues=discover(words,[0x1000],{0x2000:'unresolved import'})
        self.assertNotIn(0x2000,code)
        self.assertIn({'pc':0x2000,'reason':'unresolved import'},issues)

    def test_break_is_a_defined_explicit_trap_not_an_unresolved_boundary(self):
        from hgtool.iop_emit import emit
        words={0x1000:0x0007000d}
        code,issues=discover(words,[0x1000],{})
        self.assertFalse(issues)
        self.assertIn('IOP BREAK trap code 0x1c00',emit(code))

    def test_delay_slot_can_also_be_a_function_root(self):
        words={0x1000:0x08000800,0x1004:0,0x1008:0x03e00008,0x100c:0,0x2000:0x0000000c}
        code,_=discover(words,[0x1004,0x1000],{})
        self.assertIn(0x1008,code)

    def test_ee_extensions_not_accepted_as_iop(self):
        for word in (0x64020001,0x0080102d,0x70000428,0x42000018):
            self.assertEqual(decode(0,word).name,'unsupported')

    def test_iop_mtc0_status_is_modeled(self):
        instruction=decode(0x1000,0x40806000) # mtc0 $zero, Status
        self.assertEqual(instruction.name,'mtc0')
        from hgtool.iop_emit import operation
        self.assertEqual(operation(instruction),'s.write_cop0(12,s.r(0));')

    def test_manual_callback_targets_expand_reachability(self):
        words={0x1000:0x0080f809,0x1004:0,0x1008:0x03e00008,0x100c:0,
               0x2000:0x03e00008,0x2004:0}
        code,issues=discover(words,[0x1000],{},indirect_targets={0x1000:[0x2000]})
        self.assertIn(0x2000,code)
        self.assertFalse(issues)
        # The emitter still takes the target from the guest register at runtime.
        from hgtool.iop_emit import emit
        self.assertIn('target=s.r(4)',emit(code))

    def test_syscall_selector_evidence_requires_a_proven_straight_line_value(self):
        from hgtool.iop_build import static_syscall_evidence
        words={0x1000:0x24020020,0x1004:0x0000000c,
               0x2000:0x8c020000,0x2004:0x0000000c}
        code={pc:decode(pc,word) for pc,word in words.items()}
        issues=[{'pc':0x1004,'reason':'syscall'},{'pc':0x2004,'reason':'syscall'}]
        self.assertEqual(static_syscall_evidence(code,issues),[{'pc':0x1004,'number':0x20}])

    def test_proven_kmode_syscall_lowers_to_checked_aot_transfer(self):
        from hgtool.iop_emit import emit
        cpp=emit({},native={0x1004:'syscall_invoke_in_kmode'})
        self.assertIn('function=s.r(4)',cpp)
        self.assertIn('const auto a0=s.r(5),a1=s.r(6),a2=s.r(7)',cpp)
        self.assertIn('s.pc=function',cpp)
        self.assertIn('invalid native INTRMAN kernel callback',cpp)

    def test_threadman_reschedule_lowers_to_native_scheduler_boundary(self):
        from hgtool.iop_emit import emit
        cpp=emit({},native={0x1004:'syscall_threadman_reschedule'})
        self.assertIn('s.threadman_reschedule(s.r(7))',cpp)
