"""Validate static IOP placement and LOADCORE-compatible local library bindings."""
from pathlib import Path
import re
import tomllib
from .iop_source import read_module
from .relocate import relocate_sections
from .iop_decode import decode
import struct


def integer(value,alignment=1):
    if type(value) is not int or not 0<=value<2**32 or value%alignment:
        raise ValueError('expected nonnegative aligned IOP configuration integer')
    return value


def plan(path):
    path=Path(path).resolve();cfg=tomllib.loads(path.read_text(encoding='utf-8'))
    required={'schema_version','ram_size','reserved_low','reserved_high','modules'}
    if not required<=set(cfg) or set(cfg)-required-{'native_imports','library_providers'} or cfg['schema_version']!=1:
        raise ValueError('invalid IOP placement configuration')
    native_imports={}
    allowed={('intrman',0x102,n):handler for n,handler in {
        4:'register_interrupt',5:'release_interrupt',6:'enable_interrupt',7:'disable_interrupt',8:'disable_cpu_interrupts',9:'enable_cpu_interrupts',14:'invoke_in_kmode',
        17:'suspend_interrupts',18:'resume_interrupts',23:'query_interrupt_context',
        28:'set_new_context_callback',30:'set_should_preempt_callback'}.items()}
    allowed.update({('dmacman',0x102,n):handler for n,handler in {4:'set_dma_madr',5:'get_dma_madr',6:'set_dma_bcr',8:'set_dma_chcr',9:'get_dma_chcr'}.items()})
    allowed.update({('dmacman',0x102,14):'set_dma_priority_control',('dmacman',0x102,15):'get_dma_priority_control',
                    ('dmacman',0x102,28):'set_slice_dma',('dmacman',0x102,32):'start_dma',
                    ('dmacman',0x102,33):'set_dma_priority',('dmacman',0x102,34):'enable_dma_channel',
                    ('dmacman',0x102,35):'disable_dma_channel'})
    allowed.update({('heaplib',0x101,n):handler for n,handler in {
        4:'create_heap',5:'delete_heap',6:'alloc_heap_memory',7:'free_heap_memory',8:'heap_total_free_size'}.items()})
    allowed.update({('secrman',0x103,4):'set_secrman_mc_command_callback',
                    ('secrman',0x103,5):'set_secrman_mc_devid_callback',
                    ('secrman',0x103,6):'authenticate_card'})
    allowed.update({('vblank',0x101,4):'wait_vblank_start',('vblank',0x101,5):'wait_vblank_end'})
    for item in cfg.get('native_imports',[]):
        if set(item)!={'library','version','ordinal','handler'}:raise ValueError('invalid native IOP import fields')
        key=item['library'],integer(item['version']),integer(item['ordinal'])
        if key in native_imports or allowed.get(key)!=item['handler']:raise ValueError('invalid/duplicate native IOP import ABI')
        native_imports[key]=item['handler']
    low,high,ram=(integer(cfg[k],16) for k in ('reserved_low','reserved_high','ram_size'))
    if not 0<low<high<ram or ram!=0x200000:raise ValueError('invalid IOP RAM/reserved boundaries')
    modules=[];ranges=[];names=set()
    for item in cfg['modules']:
        data,m,source=read_module(path,item)
        name=item['name']
        if not re.fullmatch('[a-z][a-z0-9_]*',name) or name in names:raise ValueError('invalid or duplicate IOP module name')
        names.add(name);base=integer(item['base'],16)
        if not m['segments']:raise ValueError('IOP placement requires load segments')
        # LOADCORE places a 0x30-byte module header before the relocated image.
        # Reserve its complete allocation, including BSS and rounded tail.
        allocation_start=base-0x30
        allocation_end=base+((max(s['address']+s['memory_size'] for s in m['segments'])+15)&~15)
        if allocation_start<low or allocation_end>high or any(allocation_start<y and x<allocation_end for x,y in ranges):
            raise ValueError('IOP module allocations overlap or extend into reserved memory')
        segment_ranges=[]
        for s in m['segments']:
            a,b=base+s['address'],base+s['address']+s['memory_size']
            if a<low or b>high or a==b or any(a<y and x<b for x,y in segment_ranges):
                raise ValueError('IOP modules overlap or extend into reserved memory')
            segment_ranges.append((a,b))
        ranges.append((allocation_start,allocation_end))
        executable=lambda offset:any(s['type']==1 and s['flags']&4 and s['address']<=offset
                                     and offset+4<=s['address']+s['size'] for s in m['sections'])
        if m['entry']%4 or not executable(m['entry']):raise ValueError('non-executable IOP module entry')
        roots={m['entry']};function_names=set()
        for f in item['functions']:
            if set(f)!={'name','offset'} or not re.fullmatch('[A-Za-z_][A-Za-z0-9_]*',f['name']) or f['name'] in function_names:
                raise ValueError('invalid manual IOP function')
            offset=integer(f['offset'],4)
            if not executable(offset):raise ValueError('manual IOP function outside executable sections')
            roots.add(offset);function_names.add(f['name'])
        for table in m['exports']:
            for export in table['exports']:
                if not executable(export['target']):raise ValueError('IOP export outside executable sections')
                roots.add(export['target'])
        # Exercise relocation validation before this module can enter a future build.
        relocated=relocate_sections(data,m,base)
        for section in relocated:
            if not any(base+s['address']<=section['address'] and section['address']+len(section['data'])
                       <=base+s['address']+s['file_size'] for s in m['segments']):
                raise ValueError('IOP allocatable section outside file-backed load segment')
        hints={};hint_refs={}
        for hint in item.get('indirect_targets',[]):
            if set(hint) not in ({'site','targets'},{'site','table_offset','table_count'},{'site','target_refs'}):raise ValueError('invalid IOP indirect-target fields')
            site=integer(hint['site'],4)
            if site in hints or not executable(site):raise ValueError('duplicate or non-executable IOP indirect site')
            section=next(s for s in relocated if s['address']<=base+site<s['address']+len(s['data']))
            instruction=decode(base+site,struct.unpack_from('<I',section['data'],base+site-section['address'])[0])
            if instruction.name not in ('jr','jalr'):raise ValueError('IOP indirect site must be jr or jalr')
            if 'target_refs' in hint:
                refs=hint['target_refs']
                if not isinstance(refs,list) or not refs:raise ValueError('IOP indirect target references must be a nonempty list')
                parsed=[]
                for ref in refs:
                    if not isinstance(ref,dict) or set(ref)!={'module','offset'} or not re.fullmatch('[a-z][a-z0-9_]*',ref['module']):
                        raise ValueError('invalid IOP indirect target reference')
                    parsed.append((ref['module'],integer(ref['offset'],4)))
                if len(set(parsed))!=len(parsed):raise ValueError('duplicate IOP indirect target reference')
                hint_refs[site]=parsed;targets=[]
            elif 'targets' in hint:
                if not isinstance(hint['targets'],list) or not hint['targets']:raise ValueError('IOP indirect targets must be a nonempty list')
                targets=[integer(t,4) for t in hint['targets']]
                if len(set(targets))!=len(targets):raise ValueError('duplicate IOP indirect target')
            else:
                offset=integer(hint['table_offset'],4);count=integer(hint['table_count'])
                if not 0<count<=4096:raise ValueError('IOP indirect table count outside limit')
                table=next((s for s in relocated if s['address']<=base+offset and base+offset+count*4<=s['address']+len(s['data'])),None)
                if table is None:raise ValueError('IOP indirect table outside file-backed section')
                targets=sorted({struct.unpack_from('<I',table['data'],base+offset-table['address']+n*4)[0]-base for n in range(count)})
            if any(t%4 or not executable(t) for t in targets):raise ValueError('invalid IOP indirect target')
            if targets:hints[site]=targets
        hooks={}
        for f in item.get('entry_hooks',[]):
            if set(f)!={'offset','handler'} or f['handler']!='prepare_buffered_module':
                raise ValueError('invalid IOP entry hook')
            offset=integer(f['offset'],4)
            if offset in hooks or not executable(offset):raise ValueError('invalid IOP entry hook offset')
            hooks[base+offset]=f['handler'];roots.add(offset)
        native={}
        for f in item.get('native_functions',[]):
            if set(f)!={'offset','handler'} or f['handler'] not in ('delay_thread','change_thread_priority','refer_thread_status','refer_event_status','irefer_event_status','create_semaphore','signal_semaphore','wait_semaphore','print_console','get_thread_id','sleep_thread','wakeup_thread','create_thread','delete_thread','exit_thread','start_thread','flush_instruction_cache','flush_data_cache','get_system_status_flag','create_event_flag','set_event_flag','clear_event_flag','wait_event_flag','cdvd_open','cdvd_close','cdvd_read','cdvd_lseek','alloc_system_memory','free_system_memory','link_static_iop_module'):
                raise ValueError('invalid IOP native service adapter')
            offset=integer(f['offset'],4)
            if offset in native or not executable(offset):raise ValueError('invalid IOP native function offset')
            native[offset]=f['handler']
        for f in item.get('syscall_handlers',[]):
            allowed_syscalls={(12,'invoke_in_kmode'),(32,'threadman_reschedule')}
            if set(f)!={'site','number','handler'} or (f['number'],f['handler']) not in allowed_syscalls:
                raise ValueError('invalid IOP syscall handler')
            site=integer(f['site'],4)
            if site in native or not executable(site) or not executable(site-4):
                raise ValueError('invalid IOP syscall handler site')
            current=next(s for s in relocated if s['address']<=base+site<s['address']+len(s['data']))
            previous=next(s for s in relocated if s['address']<=base+site-4<s['address']+len(s['data']))
            syscall=decode(base+site,struct.unpack_from('<I',current['data'],base+site-current['address'])[0])
            selector=decode(base+site-4,struct.unpack_from('<I',previous['data'],base+site-4-previous['address'])[0])
            if syscall.name!='syscall' or selector.name!='addiu' or selector.rs or selector.rt!=2 or selector.immediate!=f['number']:
                raise ValueError('IOP syscall handler lacks exact local selector evidence')
            native[site]='syscall_'+f['handler']
        modules.append({'name':name,'base':base,'roots':[base+r for r in sorted(roots)],'inventory':m,'source':source,
                        'native_functions':{base+offset:handler for offset,handler in native.items()},'entry_hooks':hooks,
                        'indirect_targets':{base+site:[base+t for t in targets] for site,targets in hints.items()},
                        'indirect_target_refs':{base+site:refs for site,refs in hint_refs.items()}})
    module_by_name={m['name']:m for m in modules}
    for module in modules:
        for site,refs in module['indirect_target_refs'].items():
            targets=[]
            for target_name,offset in refs:
                target_module=module_by_name.get(target_name)
                if target_module is None:raise ValueError('IOP indirect target references unknown module')
                executable=any(s['type']==1 and s['flags']&4 and s['address']<=offset
                               and offset+4<=s['address']+s['size'] for s in target_module['inventory']['sections'])
                if not executable:raise ValueError('IOP indirect target reference is not executable')
                target=target_module['base']+offset
                targets.append(target)
                target_module['roots'].append(target)
            module['indirect_targets'][site]=targets
    for module in modules:module['roots']=sorted(set(module['roots']))
    providers={}
    for m in modules:
        for table in m['inventory']['exports']:
            key=table['name'],table['version']
            providers.setdefault(key,[]).append((m,{e['ordinal']:e['target'] for e in table['exports']}))
    selections={}
    for choice in cfg.get('library_providers',[]):
        if set(choice)!={'library','version','module'}:raise ValueError('invalid IOP provider selection fields')
        key=choice['library'],integer(choice['version'])
        if key in selections:raise ValueError('duplicate IOP provider selection')
        selected=[p for p in providers.get(key,[]) if p[0]['name']==choice['module']]
        if len(selected)!=1:raise ValueError('selected IOP provider does not export exact library version')
        selections[key]=selected
    bindings=[];unresolved=[];native_bindings=[]
    for m in modules:
        for library in m['inventory']['imports']:
            key=library['name'],library['version']
            if key in selections:
                candidates=selections[key]
            else:
                candidates=providers.get(key,[])
                if not candidates:
                    # Original LOADCORE link path 0x9717c calls 0x97038,
                    # comparing only the high version byte before binding.
                    # Its minor comparison belongs to export replacement.
                    candidates=[candidate for (name,version),entries in providers.items()
                                if name==key[0] and version>>8==key[1]>>8
                                for candidate in entries]
            provider=candidates[0] if len(candidates)==1 else None
            for entry in library['imports']:
                edge={'module':m['name'],'stub':m['base']+entry['stub'],'library':library['name'],
                      'version':library['version'],'ordinal':entry['ordinal']}
                native_handler=native_imports.get((library['name'],library['version'],entry['ordinal']))
                if native_handler:
                    if candidates:raise ValueError('native IOP import conflicts with a supplied provider')
                    edge['handler']=native_handler;native_bindings.append(edge)
                elif provider and entry['ordinal'] in provider[1]:
                    edge.update(provider=provider[0]['name'],target=provider[0]['base']+provider[1][entry['ordinal']])
                    bindings.append(edge)
                else:
                    edge['reason']=('ambiguous compatible providers' if len(candidates)>1 else 'no compatible major export')+'; must trap until explicitly implemented'
                    unresolved.append(edge)
    return {'ram_size':ram,'reserved_low':low,'reserved_high':high,
            'modules':[{'name':m['name'],'base':m['base'],'roots':m['roots'],
                        'sha256':m['inventory']['sha256'],'segments':m['inventory']['segments'],'source':m['source'],
                        'native_functions':m['native_functions'],'entry_hooks':m['entry_hooks'],
                        'indirect_targets':m['indirect_targets']} for m in modules],
            'bindings':bindings,'native_bindings':native_bindings,'unresolved_imports':unresolved}
