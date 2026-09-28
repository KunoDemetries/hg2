"""Identity-checked standalone or ROMDIR-contained module inputs."""
from pathlib import Path
import hashlib
from .irx import inspect_irx_data
from .romdir import RebootImage


def read_module(config, item):
    required={'name','path','sha256','base','functions'}
    region='offset' in item or 'size' in item
    if not required<=set(item) or set(item)-required-{'member','offset','size','container_sha256','indirect_targets','native_functions','syscall_handlers','entry_hooks'} or (('member' in item or region)!=('container_sha256' in item)):
        raise ValueError('invalid IOP module fields')
    if region and ('member' in item or 'offset' not in item or 'size' not in item):
        raise ValueError('invalid embedded IOP source fields')
    path=Path(config).parent/item['path']
    if region:
        offset,size=item['offset'],item['size']
        if type(offset)!=int or type(size)!=int or offset<0 or not 0<size<=16*1024*1024 or offset+size>path.stat().st_size:
            raise ValueError('invalid embedded IOP source extent')
        with path.open('rb') as stream:
            digest=hashlib.file_digest(stream,'sha256').hexdigest()
            if digest!=item['container_sha256']:raise ValueError('IOP container identity mismatch')
            stream.seek(offset);data=stream.read(size)
        source={'offset':offset,'size':size,'sha256':digest,'region':True}
    elif 'member' in item:
        image=RebootImage(path)
        if image.sha256!=item['container_sha256']:
            raise ValueError('IOP container identity mismatch')
        if item['member'] not in image.entries:raise ValueError('missing IOP container member')
        entry=image.entries[item['member']];data=image.read(item['member'])
        source={'offset':entry['offset'],'size':entry['size'],'sha256':image.sha256,'member':item['member']}
    else:
        if path.stat().st_size>16*1024*1024:raise ValueError('IOP module size limit exceeded')
        data=path.read_bytes();source={'offset':0,'size':len(data),'sha256':item['sha256']}
    inventory=inspect_irx_data(data,str(path)+('#'+item['member'] if 'member' in item else ''))
    if inventory['sha256']!=item['sha256']:raise ValueError(f'IOP module identity mismatch: {item["name"]}')
    return data,inventory,source
