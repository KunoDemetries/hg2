from pathlib import Path
import re
import tomllib


def address(value):
    if type(value) is not int or not 0 <= value < 2**32 or value % 4:
        raise ValueError(f"expected aligned 32-bit integer address, got {value!r}")
    return value


def load(path, elf_type):
    path = Path(path).resolve()
    cfg = tomllib.loads(path.read_text(encoding='utf-8'))
    if cfg.get('schema_version') != 1:
        raise ValueError('unsupported config schema_version')
    unknown = set(cfg) - {'schema_version', 'image', 'analysis', 'functions', 'indirect_targets', 'data_ranges', 'overlays', 'callback_bindings', 'vu1_programs'}
    if unknown:
        raise ValueError(f'unknown config keys: {sorted(unknown)}')
    for table, allowed in [('image', {'path', 'sha256'}), ('analysis', {'max_instructions','returning_syscalls'})]:
        if set(cfg.get(table, {})) - allowed:
            raise ValueError(f'unknown keys in {table}')
    image = elf_type.read(path.parent / cfg['image']['path'])
    if image.sha256 != cfg['image']['sha256']:
        raise ValueError('executable SHA-256 does not match config; do not use addresses from another revision')
    overlay_names=set()
    for o in cfg.get('overlays',[]):
        if set(o)-{'name','source','address','size','entry_offsets'} or not {'name','source','address','size'}<=set(o):
            raise ValueError('invalid overlay fields')
        if not re.fullmatch('[A-Za-z_][A-Za-z0-9_]*',o['name']) or o['name'] in overlay_names:
            raise ValueError('invalid or duplicate overlay name')
        overlay_names.add(o['name'])
        for key in ('source','address','size'): address(o[key])
        if o['size']==0 or o['source']+o['size']>2**32 or o['address']+o['size']>2**32:
            raise ValueError('invalid overlay bounds')
        for offset in o.get('entry_offsets',[0]):
            if address(offset)>=o['size']: raise ValueError('overlay entry is outside its bounds')
        image.add_overlay(o)
    vu_names=set()
    for program in cfg.get('vu1_programs',[]):
        legacy=set(program)=={'name','source','micro_address','pair_count','entries'}
        composite=set(program)=={'name','uploads','entries'}
        if not legacy and not composite:
            raise ValueError('vu1_programs requires either one source range or uploads plus entries')
        if not re.fullmatch('[A-Za-z_][A-Za-z0-9_]*',program['name']) or program['name'] in vu_names:
            raise ValueError('invalid or duplicate VU1 program name')
        vu_names.add(program['name'])
        uploads=[program] if legacy else program['uploads']
        if type(uploads) is not list or not uploads:
            raise ValueError('VU1 composite program requires a nonempty uploads list')
        covered=set()
        micro_start=0x4000;micro_end=0
        for upload in uploads:
            if not legacy and set(upload)!={'source','micro_address','pair_count'}:
                raise ValueError('VU1 upload requires source, micro_address and pair_count')
            source=address(upload['source'])
            micro=address(upload['micro_address'])
            count=upload['pair_count']
            if source%8 or micro%8 or type(count) is not int or not 1<=count<=2048 or micro//8+count>2048:
                raise ValueError('invalid VU1 program source/micro bounds')
            size=count*8
            if not any(s.address<=source and source+size<=s.address+s.file_size for s in image.segments):
                raise ValueError('VU1 program source must be wholly file-backed in one ELF segment')
            micro_start=min(micro_start,micro);micro_end=max(micro_end,micro+size)
            covered.update(range(micro,micro+size,8))
        if composite and any(address not in covered for address in range(micro_start,micro_end,8)):
            raise ValueError('VU1 composite uploads must cover one contiguous MicroMem range')
        entries=program['entries']
        if type(entries) is not list or not entries:
            raise ValueError('VU1 program entries must be a nonempty list')
        for entry in entries:
            if type(entry) is not int or entry%8 or entry not in covered:
                raise ValueError('VU1 entry must be aligned and inside configured MicroMem range')
    names, starts, ranges = set(), set(), []
    for f in cfg.get('functions', []):
        if set(f) - {'name', 'address', 'end'}:
            raise ValueError('unknown function keys')
        if not re.fullmatch('[A-Za-z_][A-Za-z0-9_]*', f['name']) or f['name'] in names:
            raise ValueError('invalid or duplicate function name')
        a = address(f['address'])
        if a in starts or not image.executable(a):
            raise ValueError('duplicate or non-executable function address')
        names.add(f['name']); starts.add(a)
        if 'end' in f:
            e = address(f['end'])
            if e <= a or not image.executable(e - 4):
                raise ValueError('invalid function end')
            if any(a < y and x < e for x, y in ranges):
                raise ValueError('overlapping function ranges')
            ranges.append((a, e))
    sites = set()
    bindings=set()
    for binding in cfg.get('callback_bindings',[]):
        if set(binding)!={'callee','argument','site'}:
            raise ValueError('callback_bindings requires callee, argument and site')
        from .decode import decode
        callee,site=address(binding['callee']),address(binding['site'])
        argument=binding['argument']
        if not image.executable(callee) or decode(site,image.word(site)).name not in ('jr','jalr'):
            raise ValueError('callback binding needs executable callee and indirect call site')
        if type(argument) is not int or not 1<=argument<=31:
            raise ValueError('callback argument must be a GPR number from 1 to 31')
        key=callee,argument,site
        if key in bindings: raise ValueError('duplicate callback binding')
        bindings.add(key)
    for item in cfg.get('indirect_targets', []):
        if set(item) not in ({'site','targets'},{'site','table_start','table_end'}):
            raise ValueError('indirect_targets requires site plus targets or table_start/table_end')
        a = address(item['site'])
        from .decode import decode
        if a in sites or decode(a, image.word(a)).name not in ('jr', 'jalr', 'eret'):
            raise ValueError('duplicate site or site is not jr/jalr/eret')
        sites.add(a)
        if 'table_start' in item:
            begin,end=address(item['table_start']),address(item['table_end'])
            if begin>=end or end-begin>65536*4:
                raise ValueError('invalid indirect pointer table bounds')
            item['targets']=list(dict.fromkeys(image.word(p) for p in range(begin,end,4)))
        if not item['targets']:
            raise ValueError('empty indirect target list')
        for t in item['targets']:
            if not image.executable(address(t)):
                raise ValueError('non-executable indirect target')
    for item in cfg.get('data_ranges', []):
        if set(item) != {'start', 'end'} or address(item['start']) >= address(item['end']):
            raise ValueError('invalid data range')
        if any(item['start'] <= a < item['end'] for a in starts | {image.entry}):
            raise ValueError('function or entry overlaps data')
    limit = cfg.get('analysis', {}).get('max_instructions', 100000)
    if type(limit) is not int or limit <= 0:
        raise ValueError('max_instructions must be a positive integer')
    for number in cfg.get('analysis',{}).get('returning_syscalls',[]):
        if type(number) is not int or not -32768 <= number <= 32767:
            raise ValueError('returning_syscalls must be signed 16-bit integers')
    return cfg, image
