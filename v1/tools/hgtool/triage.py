"""Compact, deterministic follow-up queue for an EE discovery report."""

from collections import Counter, defaultdict


def make_triage(code, report):
    """Group explicit discovery boundaries without adding targets or decoding again."""
    grouped = defaultdict(list)
    mnemonics = Counter()
    shapes = Counter()
    indirect_setups = Counter()
    indirect_loads = Counter()
    indirect_load_sites = defaultdict(list)
    for issue in report['issues']:
        pc = issue['pc']
        decoded = code.get(pc)
        mnemonic = decoded.name if decoded is not None else None
        if mnemonic:
            mnemonics[mnemonic] += 1
        item = {'pc': pc}
        if mnemonic:
            item['mnemonic'] = mnemonic
        if decoded is not None:
            item['word'] = decoded.word
        elif 'word' in issue:
            item['word'] = issue['word']
        if 'register' in issue:
            item['register'] = issue['register']
        shapes[(issue['reason'], mnemonic, issue.get('register'))] += 1
        if issue['reason']=='unresolved indirect transfer':
            prior,last=(code.get(pc-delta) for delta in (8,4))
            previous=tuple((instruction.name if instruction is not None else 'missing')
                           for instruction in (prior,last))
            indirect_setups[(mnemonic,issue.get('register'),previous)] += 1
            target_register=issue.get('register')
            loads_target=(last is not None and last.name in ('lw','ld') and
                          getattr(last,'rt',None)==target_register)
            loaded_base=(loads_target and prior is not None and prior.name in ('lw','ld') and
                         getattr(prior,'rt',None)==getattr(last,'rs',None))
            load_key=(mnemonic,target_register,
                      'loaded-base' if loaded_base else 'register-base' if loads_target else 'other')
            indirect_loads[load_key] += 1
            indirect_load_sites[load_key].append(pc)
        grouped[issue['reason']].append(item)
    groups = [
        {'reason': reason, 'count': len(items), 'sites': sorted(items, key=lambda item: item['pc'])}
        for reason, items in grouped.items()
    ]
    groups.sort(key=lambda group: (-group['count'], group['reason']))
    unresolved_shapes = [
        {'reason': reason, 'mnemonic': mnemonic, **({'register': register} if register is not None else {}),
         'count': count}
        for (reason, mnemonic, register), count in shapes.items()
    ]
    unresolved_shapes.sort(key=lambda shape: (-shape['count'], shape['reason'],
                                               shape.get('mnemonic') or '', shape.get('register', -1)))
    indirect_setup_shapes=[
        {'mnemonic':mnemonic, **({'register':register} if register is not None else {}),
         'previous':list(previous),'count':count}
        for (mnemonic,register,previous),count in indirect_setups.items()
    ]
    indirect_setup_shapes.sort(key=lambda shape: (-shape['count'],shape.get('mnemonic') or '',
                                                   shape.get('register',-1),shape['previous']))
    indirect_load_shapes=[
        {'mnemonic':mnemonic, **({'register':register} if register is not None else {}),
         'setup':setup,'count':count,'examples':sorted(indirect_load_sites[(mnemonic,register,setup)])[:3]}
        for (mnemonic,register,setup),count in indirect_loads.items()
    ]
    indirect_load_shapes.sort(key=lambda shape: (-shape['count'],shape.get('mnemonic') or '',
                                                  shape.get('register',-1),shape['setup']))
    return {
        'instruction_count': report['instruction_count'],
        'unresolved_count': len(report['issues']),
        'complete': report['complete'],
        'groups': groups,
        'unresolved_mnemonics': dict(sorted(mnemonics.items())),
        'unresolved_shapes': unresolved_shapes,
        'indirect_setup_shapes': indirect_setup_shapes,
        'indirect_load_shapes': indirect_load_shapes,
        'file_pointer_candidates': sorted(report.get('static_pointer_load_evidence', []),
                                          key=lambda item: (item['site'], item['load'], item['target'])),
        'installed_table_candidates': report.get('installed_table_evidence', []),
        'direct_member_candidates': report.get('direct_member_evidence', []),
        'missing_member_targets': sorted({item['target'] for item in
            report.get('direct_member_evidence', []) if not item['compiled']}),
        'missing_table_targets': sorted({entry['target']
            for table in report.get('installed_table_evidence', [])
            for entry in table['entries'] if not entry['compiled']}),
        'note': ('Read-only issue grouping. It does not infer targets, alter the '
                 'configured graph, or turn any boundary into a fallback. File-backed '
                 'pointer candidates are initialization evidence only. Installed '
                 'table scans are bounded candidates; verify their extent and live '
                 'use before adding roots. Adjacent tables may share a pointer run.'),
    }
