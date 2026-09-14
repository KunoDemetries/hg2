"""One-pass, read-only decoder inventory for prioritizing AOT work."""

from collections import Counter
from pathlib import Path
import tomllib

from .config import load
from .discover import discover
from .elf import Elf
from .iop_build import bundle, prepare
from .iop_plan import plan
from .triage import make_triage


def _ordered(counter):
    return [{'name':name,'count':count} for name,count in
            sorted(counter.items(),key=lambda item:(-item[1],item[0]))]


def _iop_issue_family(reason):
    if reason.startswith('IOP import '):
        return 'IOP import '+reason.split(':',1)[0].removeprefix('IOP import ')
    return reason


def summarize(ee_code,ee_report,iop_modules):
    """Combine already-decoded inventories without changing any CFG."""
    iop_words=Counter();iop_issues=Counter();iop_families=Counter();iop_opcodes=Counter();iop_by_module=Counter()
    for item in iop_modules:
        iop_words[item['module']]=item['reachable_words']
        iop_issues.update(issue['reason'] for issue in item['issues'])
        iop_families.update(_iop_issue_family(issue['reason']) for issue in item['issues'])
        iop_by_module[item['module']]=len(item['issues'])
        iop_opcodes.update({entry['name']:entry['count'] for entry in item['opcode_counts']})
    return {
        'ee':{'reachable_words':len(ee_code),'boundary_count':len(ee_report['issues']),
              'opcode_counts':_ordered(Counter(i.name for i in ee_code.values())),
              'boundary_reasons':_ordered(Counter(issue['reason'] for issue in ee_report['issues']))},
        'iop':{'module_count':len(iop_modules),'reachable_words':sum(iop_words.values()),
               'boundary_count':sum(iop_issues.values()),'words_by_module':_ordered(iop_words),
               'opcode_counts':_ordered(iop_opcodes),'boundary_reasons':_ordered(iop_issues),
               'boundary_families':_ordered(iop_families),'boundaries_by_module':_ordered(iop_by_module)},
        'note':('Read-only comprehensive decode inventory. It does not add targets, '
                'execute guest code, or supply an interpreter fallback.'),
    }


def run(ee_config,iop_config):
    """Decode all configured EE/IOP AOT roots and retain every boundary."""
    ee_config=Path(ee_config);iop_config=Path(iop_config)
    ee_cfg,image=load(ee_config,Elf)
    ee_code,ee_report=discover(image,ee_cfg)
    placement=plan(iop_config)
    iop_modules=[]
    for module in placement['modules']:
        code,_,_,_,_,report=prepare(iop_config,module['name'],placement)
        iop_modules.append({**report,'opcode_counts':_ordered(Counter(i.name for i in code.values()))})
    # Run the same configured set through the static linker once.  Keep only
    # diagnostic evidence: generated C++ is deliberately discarded here.
    names=[module['name'] for module in placement['modules']]
    _,bundle_report=bundle(iop_config,names)
    bundle_summary={key:bundle_report[key] for key in (
        'reachable_words','guarded_bindings','native_adapter_sites','residual_issue_count',
        'residual_by_reason','residual_shapes','residual_indirect_setups','residual_syscall_evidence',
        'trap_count','traps','note')}
    config=tomllib.loads(iop_config.read_text(encoding='utf-8'))
    configured={Path(item['path']).name.lower() for item in config['modules'] if 'member' not in item}
    module_root=(iop_config.parent/'../Haunting Ground (USA)/MODULES').resolve()
    available=sorted(path.name for path in module_root.glob('*.IRX'))
    return {
        'summary':summarize(ee_code,ee_report,iop_modules),
        'ee':{'report':ee_report,'triage':make_triage(ee_code,ee_report)},
        'iop':{'modules':iop_modules,'all_configured_bundle':bundle_summary,
                'unconfigured_module_files':[
            name for name in available if name.lower() not in configured]},
    }
