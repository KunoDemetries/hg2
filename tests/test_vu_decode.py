import unittest

from hgtool.vu_decode import decode_pair, decode_program, report_program
from hgtool.vu_emit import _body_lines, _lower, _pair_lines, _upper, _pipeline_accesses, _VFReadinessProof


def lower_primary(opcode, *, dest=0, it=0, is_=0, imm11=0):
    return ((opcode & 0x7f) << 25) | ((dest & 0xf) << 21) | ((it & 0x1f) << 16) | ((is_ & 0x1f) << 11) | (imm11 & 0x7ff)


def lower_imm15(opcode, imm15, *, it=0, is_=0):
    return ((opcode & 0x7f) << 25) | ((imm15 & 0x7800) << 10) | ((it & 0x1f) << 16) | ((is_ & 0x1f) << 11) | (imm15 & 0x7ff)


def special_word(opcode):
    # Sony type-3 extended operation number: op[6:2] lives in bits 10:6 and
    # op[1:0] selects one of the four SPECIAL low-six-bit encodings.
    return ((opcode & 0x7c) << 4) | 0x3c | (opcode & 3)


def lower_special(opcode, *, dest=0, it=0, is_=0, fsf=0, ftf=0):
    word=0x80000000 | special_word(opcode)
    word|=(dest & 0xf) << 21
    word|=(it & 0x1f) << 16
    word|=(is_ & 0x1f) << 11
    word|=(fsf & 3) << 21
    word|=(ftf & 3) << 23
    return word


def upper_direct(opcode, *, dest=0, ft=0, fs=0, fd=0, flags=0):
    return flags | ((dest & 0xf) << 21) | ((ft & 0x1f) << 16) | ((fs & 0x1f) << 11) | ((fd & 0x1f) << 6) | (opcode & 0x3f)


def upper_special(opcode, *, dest=0, ft=0, fs=0, flags=0):
    return flags | ((dest & 0xf) << 21) | ((ft & 0x1f) << 16) | ((fs & 0x1f) << 11) | special_word(opcode)


def pair(lower, upper=0x2ff):
    return (upper << 32) | lower


class VuDecodeTests(unittest.TestCase):
    def test_static_transform_schedule_guards_and_rejects_unproved_graphs(self):
        from dataclasses import replace
        from generate_vu_readiness import transform_program
        from hgtool.vu_emit import _static_transform_loop
        decoded=[decode_pair(n*8,word) for n,word in enumerate(transform_program())]
        graph={p.address:p for p in decoded}
        result=_static_transform_loop(graph,0,{0,120})
        self.assertTrue(result)
        unsigned_graph={n*8:decode_pair(n*8,word) for n,word in enumerate(transform_program(True))}
        self.assertTrue(_static_transform_loop(unsigned_graph,0,{0,120}))
        body='\n'.join(result)
        self.assertEqual(body.count('advance_pipeline('),4)
        self.assertEqual(body.count('fused_vu_transform('),1)
        self.assertIn('transform.sticky<<6',body)
        self.assertNotIn('require_vf(',body)
        self.assertNotIn('produced_vf(',body)
        self.assertIn('loop_cycle+19u',body)
        self.assertIn('loop_snapshot_4.store_qword',body)
        self.assertFalse(_static_transform_loop(graph,0,{0,24,120}))
        for index,field,change in [
            (4,'upper_fields',{'dest':14}),
            (7,'upper_fields',{'ft':11}), # translation must use architectural +1
            (5,'upper_fields',{'ft':15}), # shared coordinate required
            (4,'upper_fields',{'fs':26}), # coefficient overwritten by lower LQ
            (7,'upper_fields',{'fd':14}), # first result overwrites second-chain input
            (1,'lower_fields',{'is':8}), # counter not yet written: legal
            (5,'lower_fields',{'is':8}), # counter changed before memory address
            (4,'upper_fields',{'fs':25}), # MULq result at slot4 is not ready at slot5
        ]:
            mutated=dict(graph)
            original=graph[index*8]
            values=dict(getattr(original,field));values.update(change)
            mutated[index*8]=replace(original,**{field:values})
            if index==1:
                self.assertTrue(_static_transform_loop(mutated,0,{0,120}))
            else:
                self.assertFalse(_static_transform_loop(mutated,0,{0,120}))


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

    def test_decodes_required_lower_primary_fields(self):
        lq=decode_pair(0,pair(lower_primary(0x00,dest=0xa,it=7,is_=3,imm11=0x7fd)))
        self.assertEqual((lq.lower_name,lq.lower_fields),('lq',{'is':3,'it':7,'dest':0xa,'imm11':-3}))
        sq=decode_pair(0,pair(lower_primary(0x01,dest=5,it=9,is_=2,imm11=4)))
        self.assertEqual((sq.lower_name,sq.lower_fields),('sq',{'is':2,'it':9,'dest':5,'imm11':4}))
        isw=decode_pair(0,pair(lower_primary(0x05,dest=0xf,it=6,is_=4,imm11=7)))
        self.assertEqual((isw.lower_name,isw.lower_fields),('isw',{'is':4,'it':6,'dest':0xf,'imm11':7}))
        self.assertEqual(decode_pair(0,pair(lower_imm15(0x08,0x62ab,it=7,is_=3))).lower_fields['imm15'],0x62ab)
        self.assertEqual(decode_pair(0,pair(lower_imm15(0x09,0x7155,it=6,is_=2))).lower_name,'isubiu')
        self.assertEqual(decode_pair(0,pair((0x11<<25)|0xabcdef)).lower_fields,{'imm24':0xabcdef})
        self.assertEqual(decode_pair(0,pair((0x12<<25)|0x123456)).lower_fields,{'imm24':0x123456})

    def test_decodes_integer_and_control_lower_forms(self):
        for opcode,name in [(0x30,'iadd'),(0x31,'isub'),(0x32,'iaddi'),(0x34,'iand'),(0x35,'ior')]:
            decoded=decode_pair(0,pair(0x80000000 | (3<<16) | (2<<11) | (0x1d<<6) | opcode))
            self.assertEqual(decoded.lower_name,name)
            self.assertEqual(decoded.lower_fields['id'],0x1d)
        self.assertEqual(decode_pair(0,pair(0x80000000|(3<<16)|(2<<11)|(0x1d<<6)|0x32)).lower_fields['imm5'],-3)
        for opcode,name in [(0x20,'b'),(0x21,'bal'),(0x28,'ibeq'),(0x29,'ibne'),(0x2c,'ibltz'),(0x2d,'ibgtz'),(0x2e,'iblez'),(0x2f,'ibgez')]:
            decoded=decode_pair(0x80,pair(lower_primary(opcode,it=5,is_=4,imm11=0x7fe)))
            self.assertEqual(decoded.lower_name,name)
            self.assertEqual(decoded.lower_fields['imm11'],-2)
            self.assertEqual(decoded.branch_target,0x78)

    def test_decodes_required_lower_special_forms(self):
        names={0x30:'move',0x31:'mr32',0x34:'lqi',0x35:'sqi',0x36:'lqd',0x37:'sqd',
               0x38:'div',0x39:'sqrt',0x3a:'rsqrt',0x3b:'waitq',0x3c:'mtir',0x3d:'mfir',
               0x3e:'ilwr',0x3f:'iswr',0x68:'xtop',0x69:'xitop',0x6c:'xgkick'}
        for opcode,name in names.items():
            decoded=decode_pair(0,pair(lower_special(opcode,dest=8,it=7,is_=6)))
            self.assertEqual(decoded.lower_name,name,(hex(opcode),decoded))
        div=decode_pair(0,pair(lower_special(0x38,it=9,is_=4,fsf=2,ftf=1)))
        self.assertEqual(div.lower_fields,{'is':4,'it':9,'fsf':2,'ftf':1,'fs':4,'ft':9})
        self.assertEqual(decode_pair(0,pair(lower_special(0x68,it=2))).lower_fields,{'it':2})
        self.assertEqual(decode_pair(0,pair(lower_special(0x6c,is_=11))).lower_fields,{'is':11})
        self.assertEqual(decode_pair(0,pair(0x8000033c)).lower_name,'nop')

    def test_aot_store_operands_follow_sony_fs_it_field_order(self):
        sq=decode_pair(0,pair(lower_primary(0x01,dest=0xf,it=4,is_=11,imm11=0x7fe)))
        self.assertEqual(_lower(sq),[
            'v.vu1.store_qword(v.vu_mem,v.vu_mem_defined,11,4,-2,15);'])
        sqi=decode_pair(0,pair(lower_special(0x35,dest=0xf,it=4,is_=9)))
        self.assertEqual(_lower(sqi),[
            'v.vu1.store_qword_increment(v.vu_mem,v.vu_mem_defined,9,4,15);'])
        move=decode_pair(0,pair(lower_special(0x30,dest=1,it=10,is_=9)))
        self.assertEqual(_lower(move),['v.vu1.move_vector(10,9,1);'])
        ilw=decode_pair(0,pair(lower_primary(0x04,dest=8,it=1,is_=3,imm11=4)))
        self.assertEqual(_lower(ilw),[
            'v.vu1.load_integer(v.vu_mem,v.vu_mem_defined,1,3,4,8);'])

    def test_decodes_required_upper_forms(self):
        direct={0x00:'addx',0x01:'addy',0x02:'addz',0x03:'addw',0x08:'maddx',0x09:'maddy',
                0x0a:'maddz',0x0b:'maddw',0x10:'maxx',0x11:'maxy',0x12:'maxz',0x13:'maxw',
                0x18:'mulx',0x19:'muly',0x1a:'mulz',0x1b:'mulw',0x1c:'mulq',0x1f:'minii',0x20:'addq',0x22:'addi',
                0x28:'add',0x29:'madd',0x2a:'mul',0x2c:'sub'}
        for opcode,name in direct.items():
            decoded=decode_pair(0,pair(0x8000033c,upper_direct(opcode,dest=0xb,ft=7,fs=4,fd=3)))
            self.assertEqual(decoded.upper_name,name)
            self.assertEqual(decoded.upper_fields,{'dest':0xb,'ft':7,'fs':4,'fd':3})
        special={0x00:'addax',0x01:'adday',0x02:'addaz',0x03:'addaw',
                 0x08:'maddax',0x09:'madday',0x0a:'maddaz',0x0b:'maddaw',
                 0x10:'itof0',0x11:'itof4',0x12:'itof12',0x13:'itof15',
                 0x14:'ftoi0',0x15:'ftoi4',0x16:'ftoi12',0x17:'ftoi15',
                 0x18:'mulax',0x19:'mulay',0x1a:'mulaz',0x1b:'mulaw'}
        for opcode,name in special.items():
            decoded=decode_pair(0,pair(0x8000033c,upper_special(opcode,dest=0xd,ft=6,fs=5)))
            self.assertEqual(decoded.upper_name,name,(hex(opcode),decoded))
            self.assertEqual(decoded.upper_fields,{'dest':0xd,'ft':6,'fs':5})
        self.assertEqual(_upper(decode_pair(0,pair(0x8000033c,upper_special(0x12,dest=0xc,ft=6,fs=5)))),
                         ['v.vu1.convert_integer(6,5,12,12);'])
        self.assertEqual(_upper(decode_pair(0,pair(0x8000033c,upper_special(0x17,dest=0xc,ft=6,fs=5)))),
                         ['v.vu1.convert_fixed(6,5,12,15);'])
        clip=decode_pair(0,pair(0x8000033c,upper_special(0x1f,ft=9,fs=8)))
        self.assertEqual((clip.upper_name,clip.upper_fields),('clip',{'fs':8,'ft':9}))
        nop=decode_pair(0,pair(0x8000033c,upper_special(0x2f)))
        self.assertEqual((nop.upper_name,nop.upper_fields),('nop',{}))
        mfp=decode_pair(0,pair(lower_special(0x64,dest=1,it=10)))
        self.assertEqual((mfp.lower_name,mfp.lower_fields),('mfp',{'dest':1,'ft':10}))
        erleng=decode_pair(0,pair(lower_special(0x73,is_=9)))
        self.assertEqual((erleng.lower_name,erleng.lower_fields),('erleng',{'fs':9}))

    def test_aot_max_broadcast_matches_sony_selection_and_preserves_flags(self):
        decoded=decode_pair(0,pair(0x8000033c,upper_direct(0x10,dest=0xe,ft=0,fs=13,fd=10)))
        self.assertEqual(decoded.upper_name,'maxx')
        lines=_upper(decoded)
        self.assertEqual(lines[0],'const auto max_scalar_raw_0000=v.vu1.read_vf(0,0);')
        self.assertIn('max_source_raw_0000_0=v.vu1.read_vf(13,0)',lines[2])
        self.assertIn('max_source_raw_0000_1=v.vu1.read_vf(13,1)',lines[5])
        self.assertIn('max_source_raw_0000_2=v.vu1.read_vf(13,2)',lines[8])
        self.assertNotIn('max_source_raw_0000_3','\n'.join(lines))
        self.assertIn('hg::Vu1State::float_less(max_scalar_0000,max_source_0000_0)?max_source_0000_0:max_scalar_0000',lines[4])
        self.assertFalse(any('status' in line or 'mac' in line or 'clip' in line for line in lines))
        addi=decode_pair(0x5d8,pair(0x8000033c,upper_direct(0x22,dest=1,fs=0,fd=21)))
        addi_lines=_upper(addi)
        self.assertEqual(addi.upper_name,'addi')
        self.assertTrue(any('read_vf(0,3),v.vu1.i' in line for line in addi_lines))
        self.assertTrue(any('update_arithmetic_flags' in line for line in addi_lines))

    def test_immediate_lower_does_not_hide_upper_decode(self):
        decoded=decode_pair(0,pair(0x3f800000,upper_special(0x11,ft=2,fs=1,flags=0x80000000)))
        self.assertEqual((decoded.lower_name,decoded.lower_fields),('immediate',{'value':0x3f800000}))
        self.assertEqual(decoded.upper_name,'itof4')
        lines=_pair_lines(decoded)
        joined='\n'.join(lines)
        self.assertLess(joined.index('v.vu1.convert_integer('),joined.index('v.vu1.i=0x3f800000u;'))
        self.assertNotIn('runtime operation not implemented: lower immediate',joined)

    def test_same_pair_sq_reads_pre_upper_vf_snapshot(self):
        decoded=decode_pair(0x750,pair(
            lower_primary(0x01,dest=0xe,it=4,is_=25,imm11=0),
            upper_direct(0x0b,dest=0xe,ft=0,fs=21,fd=25)))
        self.assertEqual((decoded.lower_name,decoded.upper_name),('sq','maddw'))
        lines=_pair_lines(decoded)
        joined='\n'.join(lines)
        snapshot='const auto lower_vu1_snapshot_0750=v.vu1;'
        upper='hg::vu_xyz_madd(v.vu1,25,21,0,3);'
        store='lower_vu1_snapshot_0750.store_qword(v.vu_mem,v.vu_mem_defined,25,4,0,14);'
        self.assertLess(joined.index(snapshot),joined.index(upper))
        self.assertLess(joined.index(upper),joined.index(store))
        self.assertNotIn('v.vu1.store_qword(v.vu_mem',joined)

    def test_sq_without_upper_lane_overlap_uses_current_state(self):
        # Other VF, disjoint lanes of the same VF, and discarded VF00 writes.
        for source,destination,upper_mask,store_mask in [(25,24,14,14),(25,25,1,14),(0,0,15,15)]:
            decoded=decode_pair(0x750,pair(
                lower_primary(0x01,dest=store_mask,it=4,is_=source,imm11=-1),
                upper_direct(0x0b,dest=upper_mask,ft=0,fs=21,fd=destination)))
            joined='\n'.join(_pair_lines(decoded))
            self.assertNotIn('lower_vu1_snapshot',joined)
            self.assertIn(f'v.vu1.store_qword(v.vu_mem,v.vu_mem_defined,{source},4,-1,{store_mask});',joined)
            self.assertLess(joined.index('madd'),joined.index('store_qword'))

    def test_readiness_proof_tracks_lanes_and_four_cycle_writes(self):
        proof=_VFReadinessProof()
        self.assertEqual(proof.required(1,15),15)
        proof.read(1,14);self.assertEqual(proof.required(1,15),1)
        proof.write(1,8);self.assertEqual(proof.required(1,15),9)
        for _ in range(3):proof.advance()
        self.assertEqual(proof.required(1,15),9)
        proof.advance();self.assertEqual(proof.required(1,15),1)
        proof.write(2,15);self.assertEqual(proof.required(1,14),0)

    def test_integer_readiness_unknown_reads_and_write_latencies(self):
        proof=_VFReadinessProof()
        for _ in range(8):proof.advance()
        self.assertTrue(proof.vi_required(4))
        proof.vi_read(4);self.assertFalse(proof.vi_required(4))
        proof.vi_write(4,1);self.assertTrue(proof.vi_required(4))
        proof.advance();self.assertFalse(proof.vi_required(4))
        proof.vi_write(4,4)
        for _ in range(3):
            proof.advance();self.assertTrue(proof.vi_required(4))
        proof.advance();self.assertFalse(proof.vi_required(4))
        self.assertTrue(proof.vi_required(5))
        proof.vi_write(4,4);proof.vi_read(4)
        self.assertFalse(proof.vi_required(4))

    def test_integer_readiness_keeps_entry_checks_and_wrap_fallback(self):
        words=[pair(lower_primary(0,dest=14,it=8,is_=4)),
               pair(lower_primary(0,dest=14,it=9,is_=4)),
               pair(lower_primary(0,dest=14,it=10,is_=4)),
               pair(0x8000033c,upper_special(0x2f,flags=0x40000000)),
               pair(0x8000033c)]
        pairs=[decode_pair(n*8,word) for n,word in enumerate(words)]
        body='\n'.join(_body_lines(pairs,[0,16],optimize_readiness=True))
        first=body.split('L_0000: {')[1].split('L_0008: {')[0]
        repeated=body.split('L_0008: {')[1].split('L_0010: {')[0]
        reentry=body.split('L_0010: {')[1].split('L_0018: {')[0]
        self.assertIn('    v.vu1.require_vi(4);',first)
        self.assertIn('if(!vu_ready_clock_safe)v.vu1.require_vi(4);',repeated)
        self.assertIn('    v.vu1.require_vi(4);',reentry)
        self.assertIn('if(!v.vu1.issue_cycle)vu_ready_clock_safe=false;',body)

    def test_integer_readiness_can_be_disabled_without_disabling_vf_proof(self):
        proof=_VFReadinessProof(optimize_vi_readiness=False)
        proof.vi_read(4)
        self.assertTrue(proof.vi_required(4))
        proof.vi_write(4,1);proof.advance()
        self.assertTrue(proof.vi_required(4))
        proof.read(1,15)
        self.assertEqual(proof.required(1,15),0)
        words=[pair(lower_primary(0,dest=14,it=8,is_=4)),
               pair(lower_primary(0,dest=14,it=9,is_=4)),
               pair(lower_primary(0,dest=14,it=10,is_=4)),
               pair(0x8000033c,upper_special(0x2f,flags=0x40000000)),
               pair(0x8000033c)]
        pairs=[decode_pair(n*8,word) for n,word in enumerate(words)]
        enabled='\n'.join(_body_lines(pairs,[0],optimize_readiness=True))
        disabled='\n'.join(_body_lines(pairs,[0],optimize_readiness=True,optimize_vi_readiness=False))
        self.assertEqual(disabled,enabled.replace('if(!vu_ready_clock_safe)v.vu1.require_vi','v.vu1.require_vi'))
        self.assertEqual(disabled.count('    v.vu1.require_vi(4);'),3)
        self.assertNotIn('if(!vu_ready_clock_safe)v.vu1.require_vi',disabled)
        self.assertEqual(disabled.count('v.vu1.advance_pipeline(1);'),len(words))

    def test_aot_dependencies_respect_fields_and_integer_load_latency(self):
        q=decode_pair(0,pair(lower_primary(0, dest=0xe,it=12,is_=3),
                            upper_direct(0x1a,dest=0xc,ft=11,fs=9,fd=8)))
        reads,writes,vi_reads,vi_writes=_pipeline_accesses(q)
        self.assertEqual(reads,{9:12,11:2}) # MULz.xy reads only the broadcast Z.
        self.assertEqual(writes,{8:12,12:14})
        self.assertEqual(vi_reads,{3});self.assertEqual(vi_writes,{})
        ilw=decode_pair(0,pair(lower_primary(4,dest=8,it=6,is_=3)))
        self.assertEqual(_pipeline_accesses(ilw)[3],{6:4})
        lines=_pair_lines(q)
        self.assertLess(lines.index('v.vu1.require_vf(11,2);'),
                        lines.index('v.vu1.multiply_vector(8,9,11,12,2);'))
        self.assertGreater(lines.index('v.vu1.produced_vf(12,14);'),
                           lines.index('v.vu1.load_qword(v.vu_mem,v.vu_mem_defined,12,3,0,14);'))

    def test_report_exposes_unsupported_forms(self):
        unknown_lower=pair(0x7e<<25).to_bytes(8,'little')
        unknown_upper=pair(0x8000033c,0x37).to_bytes(8,'little')
        report=report_program(unknown_lower+unknown_upper)
        self.assertEqual(report['unsupported_pairs'],[
            {'address':0,'lower':'other','upper':'nop'},
            {'address':8,'lower':'nop','upper':'other'},
        ])

    def test_aot_body_is_static_cfg_with_branch_and_end_delay_pairs(self):
        branch=lower_primary(0x28,it=1,is_=2,imm11=3)  # IBEQ -> 0x20.
        delay=0x80000000 | (3<<16) | (3<<11) | (1<<6) | 0x32  # IADDI VI3,VI3,1.
        end_upper=upper_special(0x2f,flags=0x40000000)
        nop=0x8000033c
        data=b''.join(word.to_bytes(8,'little') for word in [
            pair(branch),pair(delay),pair(nop,end_upper),pair(nop),
            pair(nop,end_upper),pair(nop),
        ])
        body='\n'.join(_body_lines(decode_program(data),[0]))
        self.assertIn('L_0000:',body)
        self.assertIn('const bool take=v.vu1.read_vi(2)==v.vu1.read_vi(1);',body)
        self.assertEqual(body.count('v.vu1.integer_add(3,3,0x0001u,false);'),1)
        self.assertNotIn('L_0008:',body)
        self.assertIn('if(take)goto L_0020;',body)
        self.assertIn('goto L_0010;',body)
        self.assertIn('v.vu1.tpc=0x0020u;',body)
        self.assertIn('v.vu1.tpc=0x0030u;',body)
        self.assertNotIn('while(',body)
        self.assertNotIn('switch(',body)
        self.assertNotIn('decode',body.lower())
