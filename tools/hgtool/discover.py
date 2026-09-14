from collections import deque, Counter
from dataclasses import asdict
from .decode import decode


def _discover(image, cfg, returning_sites=()):
    queue = deque([image.entry] + [f['address'] for f in cfg.get('functions', [])])
    queue.extend(o['address']+offset for o in cfg.get('overlays',[]) for offset in o.get('entry_offsets',[0]))
    targets = {t['site']: t['targets'] for t in cfg.get('indirect_targets', [])}
    data = [(r['start'], r['end']) for r in cfg.get('data_ranges', [])]
    ends = {f['end'] for f in cfg.get('functions', []) if 'end' in f}
    roots = {image.entry} | {f['address'] for f in cfg.get('functions', [])}
    limit = cfg.get('analysis', {}).get('max_instructions', 100000)
    returning = cfg.get('analysis', {}).get('returning_syscalls', [])
    code, issues, seen, returns = {}, [], set(), []

    def read(pc):
        if any(a <= pc < b for a, b in data) or not image.executable(pc):
            issues.append({'pc': pc, 'reason': 'edge enters data or non-executable memory'})
            return None
        if pc not in code and len(code) >= limit:
            issues.append({'pc': pc, 'reason': 'instruction budget exhausted'})
            return None
        ins = decode(pc, image.word(pc))
        code[pc] = ins
        return ins

    while queue:
        pc = queue.popleft()
        if pc in seen:
            continue
        seen.add(pc)
        if pc in ends and pc not in roots:
            issues.append({'pc': pc, 'reason': 'configured function boundary'})
            continue
        ins = read(pc)
        if ins is None:
            continue
        if ins.name in ('unsupported', 'syscall'):
            returning_site = pc in returning_sites
            if ins.name == 'syscall' and not returning_site and image.executable(pc-4):
                previous=decode(pc-4,image.word(pc-4))
                returning_site=(previous.name=='addiu' and previous.rs==0 and
                                previous.rt==3 and previous.immediate in returning)
            if ins.name == 'syscall' and returning_site:
                queue.append(pc+4)
            else:
                issues.append({'pc': pc, 'word': ins.word, 'reason': ins.name})
            continue
        if ins.name == 'break':
            # Defined terminal synchronous exception.  The emitter retains the
            # trap; it has no fall-through until CPU exception delivery exists.
            continue
        if not ins.control:
            queue.append(pc + 4)
            continue
        if ins.name == 'eret':
            queue.extend(targets.get(pc, []))
            if pc not in targets:
                issues.append({'pc': pc, 'reason': 'unresolved exception return'})
            continue
        slot = read(pc + 4)
        if slot is None or slot.control or slot.name in ('unsupported', 'syscall', 'sync'):
            issues.append({'pc': pc, 'reason': 'unsupported delay slot'})
            if ins.likely:
                queue.append(pc+8) # The not-taken path never executes the slot.
            continue
        # BREAK is a defined synchronous exception, not an unknown instruction.
        # A normal branch always executes its slot and therefore cannot continue;
        # a branch-likely can continue only on its annulled not-taken path.
        if slot.name == 'break':
            if ins.likely:
                queue.append(pc+8)
            continue
        if ins.name in ('jr', 'jalr'):
            queue.extend(targets.get(pc, []))
            if ins.name=='jr' and ins.rs==31:
                returns.append(pc)
            elif pc not in targets:
                issues.append({'pc': pc, 'reason': 'unresolved indirect transfer', 'register': ins.rs})
            if ins.name == 'jalr':
                queue.append(pc + 8)
        elif ins.name in ('j', 'jal'):
            queue.append(ins.target)
            if ins.name == 'jal':
                queue.append(pc + 8)
        else:
            queue.extend([ins.target, pc + 8])
    report = {'sha256': image.sha256, 'entry': image.entry,
              'segments': [asdict(s) for s in image.segments],
              'overlays': cfg.get('overlays',[]),
              'instruction_count': len(code),
              'mnemonics': dict(sorted(Counter(i.name for i in code.values()).items())),
              'return_sites': sorted(returns), 'issues': issues, 'complete': not issues}
    return code, report


def discover(image,cfg):
    from copy import deepcopy
    from .constants import callback_targets, static_indirect_targets, static_pointer_load_candidates, static_syscalls, installed_table_candidates, direct_member_candidates
    working=deepcopy(cfg)
    evidence={}; returning_sites=set(); syscall_evidence=[]
    while True:
        code,report=_discover(image,working,returning_sites)
        additions=0
        by_site={item['site']:item for item in working.get('indirect_targets',[])}
        for item in callback_targets(code,working):
            target=item['target']
            if target%4 or not image.executable(target):continue
            if any(r['start']<=target<r['end'] for r in working.get('data_ranges',[])):continue
            evidence[item['caller'],item['site'],target]=item
            if item['site'] not in by_site:
                hint={'site':item['site'],'targets':[]}
                working.setdefault('indirect_targets',[]).append(hint)
                by_site[item['site']]=hint
            hint=by_site[item['site']]
            if target not in hint['targets']:
                hint['targets'].append(target); additions+=1
        static_evidence=[]
        for item in static_indirect_targets(code,working):
            target=item['target']
            if target%4 or not image.executable(target):continue
            if any(r['start']<=target<r['end'] for r in working.get('data_ranges',[])):continue
            static_evidence.append(item)
            if item['site'] not in by_site:
                hint={'site':item['site'],'targets':[]}
                working.setdefault('indirect_targets',[]).append(hint)
                by_site[item['site']]=hint
            hint=by_site[item['site']]
            if target not in hint['targets']:
                hint['targets'].append(target); additions+=1
        syscall_evidence=static_syscalls(code,working)
        returning={item['site'] for item in syscall_evidence
                   if item['number'] in working.get('analysis',{}).get('returning_syscalls',[])}
        if not returning <= returning_sites:
            returning_sites.update(returning); additions+=1
        if not additions:
            report['callback_evidence']=list(evidence.values())
            report['static_indirect_evidence']=static_evidence
            report['static_pointer_load_evidence']=static_pointer_load_candidates(code,working,image)
            report['installed_table_evidence']=installed_table_candidates(code,working,image)
            report['direct_member_evidence']=direct_member_candidates(code,working,image)
            report['static_syscall_evidence']=syscall_evidence
            return code,report
