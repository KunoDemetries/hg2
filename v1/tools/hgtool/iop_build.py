"""Reachability and AOT lowering for one statically placed IOP module."""
import json
from collections import Counter, defaultdict
from pathlib import Path
import struct
import tomllib
import os
from .iop_plan import plan
from .iop_source import read_module
from .relocate import relocate_sections
from .iop_decode import decode
from .iop_emit import emit


def discover(words,roots,blocked,limit=100000,indirect_targets=None):
    indirect_targets=indirect_targets or {}
    pending=list(roots);code={};issues=[];visited=set()
    while pending:
        pc=pending.pop()
        if pc in visited:continue
        visited.add(pc)
        if pc in blocked:
            issues.append({'pc':pc,'reason':blocked[pc]});continue
        if pc not in words:
            issues.append({'pc':pc,'reason':'outside executable module words'});continue
        if len(code)>=limit:raise ValueError('IOP discovery limit exceeded')
        i=decode(pc,words[pc]);code[pc]=i
        if i.name in ('unsupported','syscall'):
            issue={'pc':pc,'reason':i.name}
            issues.append(issue);continue
        if i.name=='break':continue
        if i.control:
            if pc+4 not in words or pc+4 in blocked:
                issues.append({'pc':pc+4,'reason':'missing/blocked delay slot'});continue
            slot=decode(pc+4,words[pc+4]);code[pc+4]=slot
            if slot.control or slot.name in ('unsupported','syscall'):
                issues.append({'pc':pc+4,'reason':'unsupported delay slot'});continue
            if slot.name=='break':continue
            if i.target is not None:pending.append(i.target)
            if i.name not in ('j','jr'):pending.append(pc+8)
            if pc in indirect_targets:pending.extend(indirect_targets[pc])
            elif i.name in ('jr','jalr') and not (i.name=='jr' and i.rs==31):
                issues.append({'pc':pc,'reason':'unresolved indirect transfer','register':i.rs})
        else:pending.append(pc+4)
    return code,issues


def prepare(config,name,placement=None):
    config=Path(config).resolve();placement=placement or plan(config)
    module=next((m for m in placement['modules'] if m['name']==name),None)
    if module is None:raise ValueError('module is not in IOP configuration')
    cfg=tomllib.loads(config.read_text())
    item=next(m for m in cfg['modules'] if m['name']==name)
    data,inventory,source=read_module(config,item);base=module['base']
    sections=relocate_sections(data,inventory,base);words={};patches=[]
    for section in sections:
        original=next(s for s in inventory['sections'] if s['name']==section['name'])
        for n,(word,) in enumerate(struct.iter_unpack('<I',section['data'])):
            address=section['address']+n*4
            if original['flags']&4:words[address]=word
            before=struct.unpack_from('<I',data,original['offset']+n*4)[0]
            if before!=word:patches.append((address,before,word))
    blocked={}
    for library in inventory['imports']:
        for item in library['imports']:
            blocked[base+item['stub']]=f'IOP import {library["name"]}:{item["ordinal"]} needs a runtime provider'
    # Single-module builds never jump into a different module that isn't compiled.
    native={**module['native_functions'],**{e['stub']:e['handler'] for e in placement['native_bindings'] if e['module']==name}}
    blocked={pc:reason for pc,reason in blocked.items() if pc not in native}
    code,issues=discover(words,module['roots'],{**blocked,**{pc:'native adapter' for pc in native}},indirect_targets=module['indirect_targets'])
    issues=[i for i in issues if i['reason']!='native adapter']
    report={'module':name,'base':base,'entry':base+inventory['entry'],'reachable_words':len(code),
            'issues':issues,'blocked_imports':blocked,'patch_count':len(patches),'native_functions':native,'entry_hooks':module['entry_hooks'],
            'traps':[{'pc':i.pc,'kind':'break','code':(i.word>>6)&0xfffff}
                     for _,i in sorted(code.items()) if i.name=='break'],
            'unsupported_words':[{'pc':i.pc,'word':i.word} for _,i in sorted(code.items()) if i.name=='unsupported']}
    return code,blocked,inventory,source,patches,report


def audit(config):
    placement=plan(config)
    return {'modules':[prepare(config,m['name'],placement)[-1] for m in placement['modules']],
            'note':'Independent reachability from declared roots; not execution or whole-module coverage.'}


def summarize_residual_issues(code,issues):
    """Read-only IOP boundary grouping; it never supplies indirect targets."""
    shapes=Counter();examples=defaultdict(list)
    for issue in issues:
        instruction=code.get(issue['pc'])
        mnemonic=instruction.name if instruction is not None else None
        key=(issue['reason'],mnemonic,issue.get('register'),issue.get('code'))
        shapes[key]+=1
        examples[key].append({'module':issue['module'],'pc':issue['pc']})
    result=[
        {'reason':reason, **({'mnemonic':mnemonic} if mnemonic is not None else {}),
         **({'register':register} if register is not None else {}),
         **({'code':code_value} if code_value is not None else {}), 'count':count,
         'examples':sorted(examples[(reason,mnemonic,register,code_value)],key=lambda item:(item['module'],item['pc']))[:3]}
        for (reason,mnemonic,register,code_value),count in shapes.items()
    ]
    result.sort(key=lambda item:(-item['count'],item['reason'],item.get('mnemonic',''),item.get('register',-1),item.get('code',-1)))
    return result


def summarize_indirect_setups(code,issues):
    """Group the two decoded words before indirect transfers without inferring targets."""
    shapes=Counter();examples=defaultdict(list)
    for issue in issues:
        if issue['reason']!='unresolved indirect transfer':continue
        pc=issue['pc'];history=[code.get(pc-delta) for delta in (16,12,8,4)]
        def label(instruction):
            if instruction is None:return 'missing'
            return 'nop' if instruction.name=='sll' and instruction.word==0 else instruction.name
        previous=tuple(label(i) for i in history[-2:])
        target=issue.get('register')
        meaningful=[i for i in history if i is not None and not(i.name=='sll' and i.word==0)]
        target_load=meaningful[-1] if meaningful and meaningful[-1].name=='lw' and meaningful[-1].rt==target else None
        loads_target=target_load is not None
        loaded_base=(loads_target and len(meaningful)>=2 and meaningful[-2].name=='lw'
                     and meaningful[-2].rt==target_load.rs)
        setup='loaded-base' if loaded_base else 'register-base' if loads_target else 'other'
        instruction=code.get(pc);mnemonic=instruction.name if instruction is not None else None
        key=(mnemonic,target,previous,setup);shapes[key]+=1
        examples[key].append({'module':issue['module'],'pc':pc})
    result=[{'mnemonic':mnemonic,**({'register':register} if register is not None else {}),
             'previous':list(previous),'setup':setup,'count':count,
             'examples':sorted(examples[(mnemonic,register,previous,setup)],
                               key=lambda item:(item['module'],item['pc']))[:3]}
            for (mnemonic,register,previous,setup),count in shapes.items()]
    result.sort(key=lambda item:(-item['count'],item.get('mnemonic') or '',
                                 item.get('register',-1),item['previous'],item['setup']))
    return result


def static_syscall_evidence(code,issues):
    """Recover a syscall selector only from its straight-line local setup.

    The result is triage evidence, not permission to continue past a syscall or
    to install a native service.  Unknown register writes deliberately discard
    the value rather than guessing a selector.
    """
    evidence=[]
    immediate_writes={'lui','addiu','addi','andi','ori','xori','slti','sltiu',
                      'lb','lbu','lh','lhu','lw','lwl','lwr','mfc0'}
    register_writes={'sll','srl','sra','sllv','srlv','srav','mfhi','mflo',
                     'add','addu','sub','subu','and','or','xor','nor','slt','sltu'}
    for issue in issues:
        if issue['reason']!='syscall':
            continue
        pc=issue['pc']; start=pc
        while start-4 in code and not code[start-4].control:
            start-=4
        values=[None]*32;values[0]=0
        for address in range(start,pc,4):
            instruction=code.get(address)
            if instruction is None or instruction.name in ('unsupported','syscall','break'):
                values[2]=None;break
            name=instruction.name;result=None;destination=None
            if name=='lui':
                destination=instruction.rt;result=(instruction.word&0xffff)<<16
            elif name=='addiu':
                destination=instruction.rt
                if values[instruction.rs] is not None:
                    result=(values[instruction.rs]+instruction.immediate)&0xffffffff
            elif name=='ori':
                destination=instruction.rt
                if values[instruction.rs] is not None:
                    result=values[instruction.rs]|(instruction.word&0xffff)
            elif name in ('addu','subu','and','or','xor','nor'):
                destination=instruction.rd
                left,right=values[instruction.rs],values[instruction.rt]
                if left is not None and right is not None:
                    result={'addu':left+right,'subu':left-right,'and':left&right,
                            'or':left|right,'xor':left^right,'nor':~(left|right)}[name]&0xffffffff
            elif name in immediate_writes:
                destination=instruction.rt
            elif name in register_writes:
                destination=instruction.rd
            if destination is not None and destination:
                values[destination]=result
        if values[2] is not None:
            evidence.append({'pc':pc,'number':values[2]})
    return evidence


def emit_load(inventory,source,patches,base,path='path',state='s'):
    if 'member' in source or source.get('region'):
        cpp=f'{"load_iop_buffered_region_file" if source.get("region") else "load_iop_region_file"}({state},{path},"{source["sha256"]}",{source["offset"]}u,{source["size"]}u,"{inventory["sha256"]}",{{\n'
    else:
        cpp=f'load_iop_file({state},{path},"{inventory["sha256"]}",{{\n'
    for segment in inventory['segments']:
        cpp+=f'{{{segment["offset"]}u,{base+segment["address"]}u,{segment["file_size"]}u,{segment["memory_size"]}u}},\n'
    cpp+='},{\n'
    cpp+=''.join(f'{{0x{a:08x}u,0x{before:08x}u,0x{after:08x}u}},\n' for a,before,after in patches)
    return cpp+(f'}},{base}u,48u);\n' if source.get('region') else '});\n')


def emit_exports(inventory,base):
    cpp=''
    for table in inventory['exports']:
        cpp+=f'if(std::string(library)=={json.dumps(table["name"])}) {{switch(ordinal) {{\n'
        for export in table['exports']:
            cpp+=f'case {export["ordinal"]}:return 0x{base+export["target"]:08x}u;\n'
        cpp+='default:break;}}\n'
    return cpp+'throw std::runtime_error("unknown IOP library export");\n'


def emit_tables(inventory,base):
    cpp=''
    for kind in ('imports','exports'):
        for table in inventory[kind]:
            cpp+=f'if(imports=={str(kind=="imports").lower()} && std::string(library)=={json.dumps(table["name"])}) return 0x{base+table["address"]:08x}u;\n'
    return cpp+'throw std::runtime_error("unknown IOP library table");\n'


def build(config,name):
    placement=plan(config)
    code,blocked,inventory,source,patches,report=prepare(config,name,placement)
    base=report['base'];cpp=emit(code,blocked,native=report['native_functions'],hooks=report['entry_hooks'])
    cpp+='\n#include "hg/iop_image.hpp"\nnamespace hg {\n'
    cpp+=f'const char* compiled_iop_name() {{return {json.dumps(name)};}}\n'
    cpp+=f'std::uint32_t compiled_iop_entry() {{return 0x{base+inventory["entry"]:08x}u;}}\n'
    cpp+='void load_compiled_iop(IopState& s,const std::filesystem::path& path) {\n'
    cpp+=emit_load(inventory,source,patches,base)+'}\n'
    cpp+='std::uint32_t compiled_iop_library_export(const char* library,unsigned ordinal) {\n'
    cpp+=emit_exports(inventory,base)+'}\n'
    cpp+=f'std::uint32_t compiled_iop_table(const char* module,const char* library,bool imports) {{if(std::string(module)!={json.dumps(name)}) throw std::runtime_error("unknown IOP module");\n'+emit_tables(inventory,base)+'}\n'
    cpp+=f'std::uint32_t compiled_iop_module_entry(const char* module) {{if(std::string(module)!={json.dumps(name)}) throw std::runtime_error("unknown IOP module");return compiled_iop_entry();}}\n'
    module_size=max(s['address']+s['memory_size'] for s in inventory['segments'])
    # MODLOAD reserves a 0x30-byte object header immediately before the
    # relocated IRX range.  The live SIO2MAN load independently confirms the
    # same bias between the SYSMEM allocation and LinkLibraryEntries address.
    allocation_prefix=0x30
    cpp+=f'std::uint32_t compiled_iop_module_base(const char* module) {{if(std::string(module)!={json.dumps(name)}) throw std::runtime_error("unknown IOP module");return 0x{base:08x}u;}}\n'
    cpp+=f'std::uint32_t compiled_iop_module_memory_size(const char* module) {{if(std::string(module)!={json.dumps(name)}) throw std::runtime_error("unknown IOP module");return 0x{module_size:08x}u;}}\n'
    cpp+=f'std::uint32_t compiled_iop_module_allocation_prefix(const char* module) {{if(std::string(module)!={json.dumps(name)}) throw std::runtime_error("unknown IOP module");return 0x{allocation_prefix:08x}u;}}\n'
    cpp+=f'std::uint32_t compiled_iop_high_water_mark() {{return 0x{base+module_size:08x}u;}}\n'
    cpp+=f'void link_compiled_iop_module(IopState& s,std::uint32_t address,std::uint32_t size) {{if(address!=0x{base:08x}u || !size || std::uint64_t(address)+size>0x{base+module_size:08x}u) throw std::runtime_error("unknown static IOP link range");\n'
    for edge in placement['bindings']:
        if edge['module']==name:
            cpp+=f's.store(0x{edge["stub"]:08x}u,4,0x{(0x08000000|((edge["target"]>>2)&0x03ffffff)):08x}u);s.store(0x{edge["stub"]+4:08x}u,4,0x{(0x24000000|edge["ordinal"]):08x}u);\n'
    cpp+='}\n'
    cpp+=f'std::uint32_t compiled_iop_module_export(const char* module,const char* library,unsigned ordinal) {{if(std::string(module)!={json.dumps(name)}) throw std::runtime_error("unknown IOP module");return compiled_iop_library_export(library,ordinal);}}\n'
    cpp+='std::uint32_t compiled_iop_export(unsigned ordinal) {\n'
    if len(inventory['exports'])==1:
        cpp+=f'return compiled_iop_library_export({json.dumps(inventory["exports"][0]["name"])},ordinal);\n'
    else:cpp+='(void)ordinal;throw std::runtime_error("ambiguous IOP export library");\n'
    cpp+='}\n}\n'
    return cpp,report


def bundle(config,names):
    """Compile modules together; unresolved/uninitialized imports remain hard boundaries."""
    if not names or len(names)!=len(set(names)):raise ValueError('IOP bundle requires unique module names')
    config=Path(config).resolve();placement=plan(config)
    cfg=tomllib.loads(config.read_text())
    items={m['name']:m for m in cfg['modules']}
    if any(name not in items for name in names):raise ValueError('IOP bundle module not configured')
    paths=[(config.parent/items[name]['path']).resolve() for name in names]
    root_paths=[p for p,name in zip(paths,names) if 'offset' not in items[name]] or paths
    root=Path(os.path.commonpath([str(p.parent) for p in root_paths]))
    prepared=[prepare(config,name,placement) for name in names]
    code={};blocked={}
    for c,b,*_ in prepared:code.update(c);blocked.update(b)
    linked={}
    for edge in placement['bindings']:
        if edge['module'] in names and edge['provider'] in names:
            linked[edge['stub']]=(edge['target'],0x24000000|edge['ordinal'])
    native={pc:handler for p in prepared for pc,handler in p[-1]['native_functions'].items()}
    hooks={pc:handler for p in prepared for pc,handler in p[-1]['entry_hooks'].items()}
    cpp=emit(code,blocked,linked,native,hooks)+'\n#include "hg/iop_image.hpp"\nnamespace hg {\n'
    cpp+='const char* compiled_iop_name() {return "bundle";}\n'
    cpp+='std::uint32_t compiled_iop_entry() {throw std::runtime_error("bundle requires explicit module initialization order");}\n'
    cpp+='std::uint32_t compiled_iop_module_entry(const char* module) {\n'
    for name,(_,_,_,_,_,report) in zip(names,prepared):
        cpp+=f'if(std::string(module)=={json.dumps(name)}) return 0x{report["entry"]:08x}u;\n'
    cpp+='throw std::runtime_error("unknown IOP module");}\n'
    cpp+='void link_compiled_iop_module(IopState& s,std::uint32_t address,std::uint32_t size) {\n'
    for name,(_,_,inventory,_,_,report) in zip(names,prepared):
        base=report['base'];prefix=0x30
        module_size=max(segment['address']+segment['memory_size'] for segment in inventory['segments'])
        cpp+=f'if(address==0x{base:08x}u && size && std::uint64_t(address)+size<=0x{base+module_size:08x}u) {{\n'
        if any(item['module']==name for item in placement['unresolved_imports']):
            cpp+='throw std::runtime_error("static IOP module retains unresolved imports");}\n'
        else:
            for edge in placement['bindings']:
                if edge['module']==name and edge['provider'] in names:
                    jump=0x08000000|((edge['target']>>2)&0x03ffffff)
                    cpp+=f's.store(0x{edge["stub"]:08x}u,4,0x{jump:08x}u);s.store(0x{edge["stub"]+4:08x}u,4,0x{0x24000000|edge["ordinal"]:08x}u);\n'
            cpp+='return;}\n'
    cpp+='throw std::runtime_error("unknown static IOP link range");}\n'
    cpp+='std::uint32_t compiled_iop_module_base(const char* module) {\n'
    for name,(_,_,_,_,_,report) in zip(names,prepared):
        cpp+=f'if(std::string(module)=={json.dumps(name)}) return 0x{report["base"]:08x}u;\n'
    cpp+='throw std::runtime_error("unknown IOP module");}\n'
    cpp+='std::uint32_t compiled_iop_module_memory_size(const char* module) {\n'
    for name,(_,_,inventory,_,_,_) in zip(names,prepared):
        module_size=max(s['address']+s['memory_size'] for s in inventory['segments'])
        cpp+=f'if(std::string(module)=={json.dumps(name)}) return 0x{module_size:08x}u;\n'
    cpp+='throw std::runtime_error("unknown IOP module");}\n'
    cpp+='std::uint32_t compiled_iop_module_allocation_prefix(const char* module) {\n'
    for name,(_,_,inventory,_,_,_) in zip(names,prepared):
        prefix=0x30
        cpp+=f'if(std::string(module)=={json.dumps(name)}) return 0x{prefix:08x}u;\n'
    cpp+='throw std::runtime_error("unknown IOP module");}\n'
    high_water=max(report['base']+max(segment['address']+segment['memory_size'] for segment in inventory['segments'])
                   for _,_,inventory,_,_,report in prepared)
    cpp+=f'std::uint32_t compiled_iop_high_water_mark() {{return 0x{high_water:08x}u;}}\n'
    cpp+='void load_compiled_iop(IopState& s,const std::filesystem::path& path) {\nIopState staged=s;\n'
    for p,(_,_,inventory,source,patches,report) in zip(paths,prepared):
        expression='path/'+json.dumps(Path(os.path.relpath(p,root)).as_posix())
        cpp+=emit_load(inventory,source,patches,report['base'],expression,'staged')
    cpp+='s=std::move(staged);}\n'
    cpp+='std::uint32_t compiled_iop_module_export(const char* module,const char* library,unsigned ordinal) {\n'
    for name,(_,_,inventory,_,_,report) in zip(names,prepared):
        cpp+=f'if(std::string(module)=={json.dumps(name)}) {{\n'+emit_exports(inventory,report['base'])+'}\n'
    cpp+='throw std::runtime_error("unknown compiled IOP module");}\n'
    cpp+='std::uint32_t compiled_iop_table(const char* module,const char* library,bool imports) {\n'
    for name,(_,_,inventory,_,_,report) in zip(names,prepared):
        cpp+=f'if(std::string(module)=={json.dumps(name)}) {{\n'+emit_tables(inventory,report['base'])+'}\n'
    cpp+='throw std::runtime_error("unknown IOP module");}\n'
    cpp+='std::uint32_t compiled_iop_library_export(const char*,unsigned) {throw std::runtime_error("bundle requires module-qualified exports");}\n'
    cpp+='std::uint32_t compiled_iop_export(unsigned) {throw std::runtime_error("bundle requires module-qualified exports");}\n}\n'
    linked_imports=[{'module':edge['module'],'stub':edge['stub'],'provider':edge['provider'],
                     'target':edge['target'],'ordinal':edge['ordinal']}
                    for edge in placement['bindings'] if edge['module'] in names and edge['provider'] in names]
    linked_stubs={edge['stub'] for edge in linked_imports}
    residual_issues=[]
    for name,(_,_,_,_,_,report) in zip(names,prepared):
        for issue in report['issues']:
            if issue['pc'] in linked_stubs and issue['reason'].startswith('IOP import '):continue
            # A module-qualified callback is outside the caller's image during
            # isolated discovery, but is resolved when its target module and
            # validated root are present in this combined code map.
            if issue['reason']=='outside executable module words' and issue['pc'] in code:continue
            residual_issues.append({'module':name,**issue})
    residual_by_reason=dict(sorted(Counter(issue['reason'] for issue in residual_issues).items(),
                                   key=lambda item: (-item[1],item[0])))
    residual_shapes=summarize_residual_issues(code,residual_issues)
    residual_indirect_setups=summarize_indirect_setups(code,residual_issues)
    residual_syscall_evidence=static_syscall_evidence(code,residual_issues)
    traps=[{'module':name,**trap} for name,(_,_,_,_,_,report) in zip(names,prepared)
           for trap in report['traps']]
    return cpp,{'modules':[p[-1] for p in prepared],'input_root':str(root),'reachable_words':len(code),
                'guarded_bindings':len(linked),'linked_imports':linked_imports,
                'native_adapter_sites':len(native),
                'residual_issues':residual_issues,'residual_issue_count':len(residual_issues),
                'residual_by_reason':residual_by_reason,'residual_shapes':residual_shapes,
                'residual_indirect_setups':residual_indirect_setups,
                'residual_syscall_evidence':residual_syscall_evidence,
                'traps':traps,'trap_count':len(traps),
                'note':'Shared static address space; planned imports require original linker-written stub words before transfer.'}
