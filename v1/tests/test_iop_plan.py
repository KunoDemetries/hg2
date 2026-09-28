import hashlib
import struct
import tempfile
from pathlib import Path
import unittest
from test_irx import fixture
from hgtool.iop_plan import plan


class IopPlanTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name)
        b=fixture();struct.pack_into('<I',b,28,52);struct.pack_into('<HH',b,42,32,1)
        struct.pack_into('<8I',b,52,1,0x100,0,0,68,0x100,7,16)
        struct.pack_into('<I',b,0x118,0x24000001)
        (self.root/'a.irx').write_bytes(b)
        b[0x130:0x138]=b'syntheti';struct.pack_into('<H',b,0x12c,0x102)
        (self.root/'b.irx').write_bytes(b)

    def config(self,second_base=0x20000,offset=4):
        s='schema_version=1\nram_size=0x200000\nreserved_low=0xffd0\nreserved_high=0x1f0000\n'
        for name,base in (('a',0x10000),('b',second_base)):
            digest=hashlib.sha256((self.root/(name+'.irx')).read_bytes()).hexdigest()
            s+=f'\n[[modules]]\nname="{name}"\npath="{name}.irx"\nsha256="{digest}"\nbase={base}\n'
            s+=f'functions=[{{name="manual",offset={offset}}}]\n'
        path=self.root/'config.toml';path.write_text(s);return path

    def test_binding_and_explicit_roots(self):
        p=plan(self.config())
        self.assertIn(0x10004,p['modules'][0]['roots'])
        edge=next(e for e in p['bindings'] if e['module']=='a')
        self.assertEqual(edge['stub'],0x10014)
        self.assertEqual(edge['target'],0x20008)
        self.assertEqual(edge['provider'],'b')

    def test_overlap_and_invalid_function(self):
        with self.assertRaises(ValueError):plan(self.config(second_base=0x10080))
        with self.assertRaises(ValueError):plan(self.config(offset=0xffc))

    def test_loader_header_and_reserved_boundary(self):
        # Images do not overlap; the second loader header would corrupt BSS.
        with self.assertRaises(ValueError):plan(self.config(second_base=0x10120))
        plan(self.config(second_base=0x10130))
        path=self.config();path.write_text(path.read_text().replace("reserved_low=0xffd0","reserved_low=0x10000"))
        with self.assertRaises(ValueError):plan(path)

    def test_identity_and_version_compatibility(self):
        path=self.config();b=bytearray((self.root/'b.irx').read_bytes());b[-1]^=1
        (self.root/'b.irx').write_bytes(b)
        with self.assertRaises(ValueError):plan(path)
        struct.pack_into('<H',b,0x12c,0x103);(self.root/'b.irx').write_bytes(b)
        p=plan(self.config())
        self.assertEqual(len(p['bindings']),2)
        self.assertFalse(p['unresolved_imports'])
        b=bytearray((self.root/'b.irx').read_bytes())
        struct.pack_into('<H',b,0x12c,0x203);(self.root/'b.irx').write_bytes(b)
        p=plan(self.config())
        self.assertEqual(len(p['unresolved_imports']),2)
        self.assertFalse(p['bindings'])
        b=bytearray((self.root/'b.irx').read_bytes())
        struct.pack_into('<H',b,0x12c,0x101);(self.root/'b.irx').write_bytes(b)
        p=plan(self.config())
        self.assertFalse(p['unresolved_imports'])
        self.assertEqual(len(p['bindings']),2)

    def test_container_source_and_identity(self):
        path=self.config();module=(self.root/'a.irx').read_bytes()
        container=b''.join(struct.pack('<10sHI',name,0,size) for name,size in (
            (b'RESET',0),(b'ROMDIR',80),(b'EXTINFO',0),(b'MODULE',len(module)),(b'',0)))+module
        image=self.root/'reboot.img';image.write_bytes(container)
        text=path.read_text().replace('path="a.irx"','path="reboot.img"\nmember="MODULE"\ncontainer_sha256="'+hashlib.sha256(container).hexdigest()+'"')
        path.write_text(text)
        p=plan(path);self.assertEqual(p['modules'][0]['source']['offset'],80)
        self.assertEqual(p['bindings'][0]['provider'],'b')
        # A container edit outside the member must also reject the source.
        changed=bytearray(container);changed[10]=1;image.write_bytes(changed)
        with self.assertRaises(ValueError):plan(path)

    def test_ambiguous_providers_are_never_selected(self):
        path=self.config();text=path.read_text()
        second=text[text.index('[[modules]]',text.index('[[modules]]')+1):]
        path.write_text(text+'\n'+second.replace('name="b"','name="c"').replace('base=131072','base=196608'))
        p=plan(path)
        self.assertFalse(p['bindings'])
        self.assertTrue(all('ambiguous' in edge['reason'] for edge in p['unresolved_imports']))

    def test_embedded_region_identity_and_extent(self):
        path=self.config();module=(self.root/'a.irx').read_bytes()
        container=b'prefix!!'+module+b'trailer'
        image=self.root/'game.elf';image.write_bytes(container)
        text=path.read_text().replace('path="a.irx"',f'path="game.elf"\noffset=8\nsize={len(module)}\ncontainer_sha256="{hashlib.sha256(container).hexdigest()}"')
        path.write_text(text)
        p=plan(path);self.assertEqual(p['modules'][0]['source']['offset'],8)
        self.assertTrue(p['modules'][0]['source']['region'])
        from hgtool.iop_build import bundle
        cpp,_=bundle(path,['a','b'])
        self.assertIn('load_iop_buffered_region_file',cpp)
        for changed in (text.replace('offset=8','offset=-1'),text.replace(f'size={len(module)}','size=999999'),text.replace('offset=8','member="MODULE"\noffset=8')):
            path.write_text(changed)
            with self.assertRaises(ValueError):plan(path)
        path.write_text(text);image.write_bytes(b'X'+container[1:])
        with self.assertRaises(ValueError):plan(path)

    def test_manual_indirect_configuration(self):
        path=self.config();text=path.read_text()
        text=text.replace('functions=[','indirect_targets=[{site=20,targets=[8]}]\nfunctions=[')
        path.write_text(text)
        self.assertEqual(plan(path)['modules'][0]['indirect_targets'],{0x10014:[0x10008]})
        path.write_text(text.replace('site=20','site=0'))
        with self.assertRaises(ValueError):plan(path)
        path.write_text(text.replace('targets=[8]','targets=[4096]'))
        with self.assertRaises(ValueError):plan(path)
        table=text.replace('targets=[8]','table_offset=56,table_count=1')
        path.write_text(table)
        self.assertEqual(plan(path)['modules'][0]['indirect_targets'],{0x10014:[0x10000]})
        path.write_text(table.replace('table_count=1','table_count=0'))
        with self.assertRaises(ValueError):plan(path)
        path.write_text(table.replace('table_offset=56','table_offset=4096'))
        with self.assertRaises(ValueError):plan(path)

    def test_module_qualified_indirect_configuration(self):
        path=self.config();text=path.read_text()
        text=text.replace('functions=[','indirect_targets=[{site=20,target_refs=[{module="a",offset=8}]}]\nfunctions=[',1)
        path.write_text(text)
        self.assertEqual(plan(path)['modules'][0]['indirect_targets'],{0x10014:[0x10008]})
        path.write_text(text.replace('offset=8','offset=4096',1))
        with self.assertRaisesRegex(ValueError,'not executable'):plan(path)
        path.write_text(text.replace('module="a"','module="missing"',1))
        with self.assertRaisesRegex(ValueError,'unknown module'):plan(path)

    def test_syscall_handler_requires_exact_local_selector(self):
        module=bytearray((self.root/'a.irx').read_bytes())
        struct.pack_into('<I',module,52+16,76) # Extend the synthetic load segment.
        struct.pack_into('<I',module,0x400+2*40+20,76) # Extend .text.
        struct.pack_into('<II',module,0x144,0x2402000c,0x0000000c)
        (self.root/'a.irx').write_bytes(module)
        path=self.config();text=path.read_text()
        text=text.replace('functions=[','syscall_handlers=[{site=72,number=12,handler="invoke_in_kmode"}]\nfunctions=[',1)
        path.write_text(text)
        self.assertEqual(plan(path)['modules'][0]['native_functions'][0x10048],
                         'syscall_invoke_in_kmode')
        path.write_text(text.replace('number=12','number=11',1))
        with self.assertRaisesRegex(ValueError,'invalid IOP syscall handler'):plan(path)

        # Mutating the original selector while retaining a matching identity is
        # also rejected by the decoded local-evidence check.
        module=bytearray((self.root/'a.irx').read_bytes());struct.pack_into('<I',module,0x144,0x2402000b)
        (self.root/'a.irx').write_bytes(module)
        changed=self.config();changed_text=changed.read_text()
        changed_text=changed_text.replace('functions=[','syscall_handlers=[{site=72,number=12,handler="invoke_in_kmode"}]\nfunctions=[',1)
        changed.write_text(changed_text)
        with self.assertRaisesRegex(ValueError,'lacks exact local selector evidence'):plan(changed)

    def test_threadman_reschedule_requires_selector_32(self):
        module=bytearray((self.root/'a.irx').read_bytes())
        struct.pack_into('<I',module,52+16,76)
        struct.pack_into('<I',module,0x400+2*40+20,76)
        struct.pack_into('<II',module,0x144,0x24020020,0x0000000c)
        (self.root/'a.irx').write_bytes(module)
        path=self.config();text=path.read_text()
        text=text.replace('functions=[','syscall_handlers=[{site=72,number=32,handler="threadman_reschedule"}]\nfunctions=[',1)
        path.write_text(text)
        self.assertEqual(plan(path)['modules'][0]['native_functions'][0x10048],
                         'syscall_threadman_reschedule')
        path.write_text(text.replace('number=32','number=31',1))
        with self.assertRaisesRegex(ValueError,'invalid IOP syscall handler'):plan(path)
