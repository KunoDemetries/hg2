"""Windows-only, read-only capture of verified PCSX2 guest RAM; never emulator code.

Use --locate to find RAM by several fingerprints from the user-local ELF. Capture
requires an explicitly selected host RAM base and an output directory outside the
repository. --suspend takes one coherent read-only snapshot by suspending PCSX2 for
the operation and always resuming it afterwards. No process writes or guest
execution injection occur.
"""
import argparse
import ctypes as c
from ctypes import wintypes as w
import datetime
import hashlib
import json
import os
from pathlib import Path
import struct
import sys
import time
import tomllib
from hgtool.elf import Elf
from hgtool.iop_source import read_module

IOP_TOC_DMA_SIZE=2064
IOP_TOC_DMA_BCR=0x00810004
IOP_TOC_DMA_CHCR=0x41000200
CDVDMAN_GETTOC_RETURN_OFFSET=0x70c4
# These are observed buffers that have explicit negative provenance in
# docs/ORACLE.md. A digest is retained rather than any captured game data.
REJECTED_TOC_CANDIDATE_HASHES={
    '1d830c8af4ff60b1ec36350ea25d99c4247d5d040d8d2c9acbe28011ebb9039e':
        'rejected: observed zeroed pre-initialization buffer; do not use as a TOC record',
    '6602314dc16454765b397f53e52891c3ab6f3ceeb6c7e525e3212fbdd476710a':
        'rejected: observed allocator fill pattern; do not use as a TOC record',
    'd6d66cf27e7968011610ffc670b47a06eb98e417a24e1428420dbe994397a503':
        'rejected: observed partial request-metadata initialization; do not use as a TOC record',
    'a010f2cc46699c9dd0c4c715c1b0a7612fa7bd927103f4f83120de01b59ccb69':
        'rejected: observed request-metadata transition; do not use as a TOC record',
    '5cd53d1b876331a0888365d0d143680dc0d0bd02b52062a9b2dd2c16a85ad4d9':
        'rejected: observed request-metadata transition; do not use as a TOC record',
    'e16d59a4036bc21d93f874dfb1b2b8844ac440c1031a28ad219134c232a93429':
        'rejected: observed request-metadata transition; do not use as a TOC record',
    'b54e0963af0a5cfe515672bb81c7c4e3abd35f4d3a523f2aab2404846bd55e90':
        'rejected: observed pre-GetToc command buffer; do not use as a TOC record',
    'e3ee9ebdb8b690c1cf2a63af53f016cdbcbd0a2f03f5848ee7dafb1f3e140c54':
        'rejected: invalid module-relative hypothesis selected executable code; do not use as a TOC record',
}

def extract_iop_toc_candidate(data,offset):
    """Return only the observed CDVDMAN GetToc DMA range from a full IOP map."""
    if offset<0 or len(data)<offset+IOP_TOC_DMA_SIZE:
        raise ValueError('TOC candidate range is outside captured IOP RAM')
    return data[offset:offset+IOP_TOC_DMA_SIZE]


def find_iop_toc_dma_descriptors(data,host_start=0):
    """Find exact active channel-3 descriptor triples in a read-only host range."""
    suffix=struct.pack('<II',IOP_TOC_DMA_BCR,IOP_TOC_DMA_CHCR)
    found=[];index=data.find(suffix,4)
    while index>=0:
        madr=struct.unpack_from('<I',data,index-4)[0]
        if madr%4==0 and madr+IOP_TOC_DMA_SIZE<=2*1024*1024:
            found.append({'host_address':host_start+index-4,'madr':madr})
        index=data.find(suffix,index+1)
    return found


def find_iop_cdvd_request_frames(data,guest_start=0,total_size=None):
    """Find original 2064-byte CDVD request descriptors and frame evidence."""
    total_size=len(data) if total_size is None else total_size
    if not 0<=guest_start<=total_size or guest_start+len(data)>total_size:
        raise ValueError('invalid IOP RAM window')
    prefix=struct.pack('<HH',4,0x81)
    found=[];offset=data.find(prefix)
    while offset>=0:
        if offset%4==0 and offset+0x34<=len(data):
            destination=struct.unpack_from('<I',data,offset+4)[0]
            zero_word=struct.unpack_from('<I',data,offset+8)[0]
            tail_count,tail_size=struct.unpack_from('<HH',data,offset+12)
            tail_word=struct.unpack_from('<I',data,offset+16)[0]
            saved_return=struct.unpack_from('<I',data,offset+0x30)[0]
            if (destination%4==0 and destination+IOP_TOC_DMA_SIZE<=total_size and
                    zero_word==0 and tail_count==0 and tail_size==0x84 and
                    tail_word==0):
                found.append({'frame':guest_start+offset-0x18,'descriptor':guest_start+offset,
                              'madr':destination,'saved_return':saved_return})
        offset=data.find(prefix,offset+1)
    return found


def find_iop_gettoc_request_frames(data,module_base,guest_start=0,total_size=None):
    """Find exact original CDVDMAN GetToc stack frames in IOP RAM."""
    if not 0<=module_base<2*1024*1024 or module_base%4:
        raise ValueError('invalid CDVDMAN module base')
    expected_return=module_base+CDVDMAN_GETTOC_RETURN_OFFSET
    return [frame for frame in find_iop_cdvd_request_frames(data,guest_start,total_size)
            if frame['saved_return']==expected_return]


def toc_candidate_status(digest):
    """Classify observed candidate provenance without interpreting its bytes."""
    return REJECTED_TOC_CANDIDATE_HASHES.get(
        digest,'candidate; do not use without independent command-stage provenance')


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--pid',type=int,required=True)
    p.add_argument('--elf',type=Path,default=Path('Haunting Ground (USA)/SLUS_210.75'))
    p.add_argument('--locate',action='store_true')
    p.add_argument('--locate-iop',metavar='MODULE',help='find an unrelocated IOP module anchor in host memory')
    p.add_argument('--locate-iop-toc-dma',action='store_true',help='find active GetToc channel-3 MADR/BCR/CHCR triples in read-only process memory')
    p.add_argument('--monitor-locate-iop-toc-ms',type=int,
                   help='repeat the read-only descriptor scan for 100..60000 ms and stop on the first exact hit')
    p.add_argument('--iop-base',type=lambda s:int(s,0),help='host base of the 2 MiB IOP RAM map to capture')
    p.add_argument('--iop-module',metavar='MODULE',help='configured module used to verify an IOP capture')
    p.add_argument('--iop-toc-candidate',action='store_true',help='also emit the observed 2064-byte GetToc DMA-buffer candidate')
    p.add_argument('--monitor-iop-toc-ms',type=int,help='poll the observed GetToc range for this bounded duration after its module anchor appears')
    p.add_argument('--monitor-iop-gettoc-request-ms',type=int,
                   help='find the original CDVDMAN GetToc stack frame and monitor its destination for 100..60000 ms')
    p.add_argument('--iop-toc-offset',type=lambda s:int(s,0),help='guest MADR captured at the same original GetToc command stage')
    p.add_argument('--iop-config',type=Path,default=Path('config/iop_modules.toml'))
    p.add_argument('--base',type=lambda s:int(s,0))
    p.add_argument('--out',type=Path)
    p.add_argument('--note',default='')
    p.add_argument('--suspend',action='store_true',help='temporarily suspend PCSX2 for a coherent snapshot')
    a=p.parse_args()
    if os.name!='nt': p.error('oracle capture is Windows-only; recompilation runtime remains portable')
    if a.iop_base is not None and not a.iop_module: p.error('--iop-base requires --iop-module')
    if a.iop_toc_candidate and a.iop_base is None: p.error('--iop-toc-candidate requires --iop-base and --iop-module')
    if (a.iop_toc_candidate or a.monitor_iop_toc_ms is not None) and a.iop_toc_offset is None:
        p.error('TOC capture requires --iop-toc-offset from the same original GetToc command stage')
    if a.iop_toc_offset is not None and (a.iop_toc_offset<0 or a.iop_toc_offset+IOP_TOC_DMA_SIZE>2*1024*1024):
        p.error('--iop-toc-offset range is outside IOP RAM')
    if a.monitor_iop_toc_ms is not None and (a.iop_base is None or a.out is None or a.suspend or not 100<=a.monitor_iop_toc_ms<=60000):
        p.error('--monitor-iop-toc-ms requires --iop-base, --iop-module, --out, no --suspend, and a duration from 100 to 60000')
    if a.monitor_iop_gettoc_request_ms is not None and (
            a.iop_base is None or a.iop_module!='rom_cdvdman' or a.out is None or a.suspend or
            not 100<=a.monitor_iop_gettoc_request_ms<=60000):
        p.error('--monitor-iop-gettoc-request-ms requires --iop-base, --iop-module rom_cdvdman, --out, no --suspend, and a duration from 100 to 60000')
    if a.monitor_locate_iop_toc_ms is not None and (
            not a.locate_iop_toc_dma or a.suspend or not 100<=a.monitor_locate_iop_toc_ms<=60000):
        p.error('--monitor-locate-iop-toc-ms requires --locate-iop-toc-dma, no --suspend, and a duration from 100 to 60000')
    if a.monitor_locate_iop_toc_ms is not None and a.iop_base is not None and a.out is None:
        p.error('descriptor-stage candidate capture requires --out with --iop-base')
    if not a.locate and not a.locate_iop and not a.locate_iop_toc_dma and (a.out is None or (a.base is None and a.iop_base is None)):
        p.error('capture requires --base or --iop-base, plus --out')
    kernel=c.WinDLL('kernel32',use_last_error=True)
    kernel.OpenProcess.argtypes=[w.DWORD,w.BOOL,w.DWORD]; kernel.OpenProcess.restype=w.HANDLE
    kernel.CloseHandle.argtypes=[w.HANDLE]
    kernel.ReadProcessMemory.argtypes=[w.HANDLE,c.c_void_p,c.c_void_p,c.c_size_t,c.POINTER(c.c_size_t)]
    kernel.ReadProcessMemory.restype=w.BOOL
    class MBI(c.Structure):
        _fields_=[('BaseAddress',c.c_void_p),('AllocationBase',c.c_void_p),('AllocationProtect',w.DWORD),
                  ('PartitionId',w.WORD),('RegionSize',c.c_size_t),('State',w.DWORD),('Protect',w.DWORD),('Type',w.DWORD)]
    kernel.VirtualQueryEx.argtypes=[w.HANDLE,c.c_void_p,c.POINTER(MBI),c.c_size_t]
    kernel.VirtualQueryEx.restype=c.c_size_t
    kernel.QueryFullProcessImageNameW.argtypes=[w.HANDLE,w.DWORD,w.LPWSTR,c.POINTER(w.DWORD)]
    # PROCESS_SUSPEND_RESUME is required only for --suspend.  VM_READ remains
    # the sole memory access: the oracle never writes process or guest memory.
    access=0x410 | (0x0800 if a.suspend else 0)
    handle=kernel.OpenProcess(access,False,a.pid)
    if not handle: raise OSError(c.get_last_error(),'cannot open oracle process read-only')
    suspended=False
    try:
        name=c.create_unicode_buffer(32768); size=w.DWORD(len(name))
        if not kernel.QueryFullProcessImageNameW(handle,0,name,c.byref(size)):
            raise OSError('cannot verify process identity')
        if Path(name.value).name.lower()!='pcsx2-qt.exe': raise ValueError('selected PID is not pcsx2-qt.exe')
        if a.suspend:
            ntdll=c.WinDLL('ntdll',use_last_error=True)
            ntdll.NtSuspendProcess.argtypes=[w.HANDLE]
            ntdll.NtSuspendProcess.restype=c.c_long
            ntdll.NtResumeProcess.argtypes=[w.HANDLE]
            ntdll.NtResumeProcess.restype=c.c_long
            status=ntdll.NtSuspendProcess(handle)
            if status!=0: raise OSError(status,'could not suspend oracle process')
            suspended=True
        elf=Elf.read(a.elf)
        def read(address,size):
            buf=c.create_string_buffer(size); count=c.c_size_t()
            if not kernel.ReadProcessMemory(handle,c.c_void_p(address),buf,size,c.byref(count)) or count.value!=size:
                return None
            return buf.raw
        # Only compare bytes to the game executable; never inspect emulator PE/JIT.
        probes=[elf.entry,0x100300,0x200000]
        signatures=[]
        for addr in probes:
            signatures.append((addr,b''.join(elf.word(addr+i).to_bytes(4,'little') for i in range(0,64,4))))
        def verified(base):
            return base>=0 and all(read(base+addr,len(sig))==sig for addr,sig in signatures)
        def iop_anchor(module_name):
            config=a.iop_config
            parsed=tomllib.loads(config.read_text(encoding='utf-8'))
            item=next((entry for entry in parsed['modules'] if entry['name']==module_name),None)
            if item is None: raise ValueError(f'unknown configured IOP module: {module_name}')
            data,inventory,_=read_module(config,item)
            text=next((section for section in inventory['sections'] if section['name']=='.text'),None)
            if text is None: raise ValueError(f'{module_name} has no text section')
            relocated={entry['address'] for entry in inventory['relocations']}
            # Loader relocations may modify individual MIPS words.  Select a
            # unique 64-byte text window containing none of them, so matching it
            # compares only supplied module bytes, not any emulator behavior.
            for address in range(text['address'],text['address']+text['size']-64+1,4):
                if any(word in relocated for word in range(address,address+64,4)): continue
                offset=text['offset']+address-text['address']; anchor=data[offset:offset+64]
                if data.count(anchor)==1: return address,anchor
            raise ValueError(f'no unique unrelocated anchor in {module_name}')
        if a.monitor_locate_iop_toc_ms is not None:
            # Cache the virtual-region inventory once, then spend the bounded
            # window only on read-only content scans. Overlapping chunk tails
            # are necessary for boundary-spanning triples and are deduplicated.
            regions=[];address=0
            while address<0x0000800000000000:
                mbi=MBI()
                if not kernel.VirtualQueryEx(handle,c.c_void_p(address),c.byref(mbi),c.sizeof(mbi)):break
                start=mbi.BaseAddress or 0;length=mbi.RegionSize
                if mbi.State==0x1000 and (mbi.Protect&0xff) in (2,4,8) and not(mbi.Protect&0x100) and 0x1000<=length<=0x40000000:
                    regions.append((start,length))
                if start+length<=address:break
                address=start+length
            deadline=time.perf_counter()+a.monitor_locate_iop_toc_ms/1000
            passes=0;scanned=0;started=time.perf_counter();result=None
            while time.perf_counter()<deadline and result is None:
                passes+=1;seen=set()
                for start,length in regions:
                    offset=0
                    while offset<length and time.perf_counter()<deadline:
                        data=read(start+offset,min(0x400000+63,length-offset))
                        if data:
                            scanned+=len(data)
                            for item in find_iop_toc_dma_descriptors(data,start+offset):
                                if item['host_address'] not in seen:
                                    seen.add(item['host_address']);result=item;break
                        if result is not None:break
                        offset+=0x400000
                    if result is not None:break
            candidate_metadata=None
            if result is not None and a.iop_base is not None:
                anchor_address,anchor=iop_anchor(a.iop_module)
                iop_data=read(a.iop_base,2*1024*1024)
                if iop_data is None:raise ValueError('unable to verify selected IOP RAM mapping after descriptor hit')
                anchor_offset=iop_data.find(anchor)
                if anchor_offset<0:raise ValueError('IOP RAM mapping failed supplied-module anchor verification after descriptor hit')
                madr=result['madr']
                first=read(a.iop_base+madr,IOP_TOC_DMA_SIZE)
                second=read(a.iop_base+madr,IOP_TOC_DMA_SIZE)
                if first is None or first!=second:
                    raise ValueError('descriptor-stage TOC candidate was not stable across consecutive reads')
                out=a.out.resolve();repo=Path(__file__).resolve().parent.parent
                if out==repo or repo in out.parents:raise ValueError('oracle dumps must stay outside repository')
                out.mkdir(parents=True,exist_ok=True)
                stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
                target=out/f'toc-descriptor-{stamp}.bin';target.write_bytes(first)
                digest=hashlib.sha256(first).hexdigest()
                candidate_metadata={'file':str(target),'sha256':digest,'status':toc_candidate_status(digest),
                    'size':len(first),'guest_madr':hex(madr),'host_iop_base':hex(a.iop_base),
                    'iop_module':a.iop_module,'iop_anchor_module_offset':hex(anchor_address),
                    'iop_anchor_guest_offset':hex(anchor_offset),
                    'iop_module_guest_base':hex(anchor_offset-anchor_address),
                    'read_consistency':'two consecutive identical ReadProcessMemory results immediately after descriptor hit'}
                target.with_suffix('.json').write_text(json.dumps(candidate_metadata,indent=2)+'\n')
            metadata={'pid':a.pid,'process':name.value,'duration_limit_ms':a.monitor_locate_iop_toc_ms,
                      'elapsed_ms':round((time.perf_counter()-started)*1000,3),'region_count':len(regions),
                      'passes':passes,'scanned_bytes':scanned,'descriptor':result,
                      'candidate_capture':candidate_metadata,
                      'access':'read-only; no process or guest writes'}
            print(json.dumps(metadata,indent=2))
            if result is None:raise ValueError('no active GetToc DMA descriptor observed during bounded monitor')
            return
        if a.locate or a.locate_iop or a.locate_iop_toc_dma:
            anchor_address=anchor=None
            if a.locate_iop:
                anchor_address,anchor=iop_anchor(a.locate_iop)
                print(f'IOP anchor {a.locate_iop}: module offset 0x{anchor_address:x}',flush=True)
            address=0; found=set(); regions=0; scanned=0; hits=0; large=[]
            while address<0x0000800000000000:
                mbi=MBI()
                if not kernel.VirtualQueryEx(handle,c.c_void_p(address),c.byref(mbi),c.sizeof(mbi)): break
                start=mbi.BaseAddress or 0; length=mbi.RegionSize
                if mbi.State==0x1000 and length>=0x1000000: large.append((hex(start),hex(length),hex(mbi.Protect),hex(mbi.Type)))
                # Exclude all executable pages. EE RAM is read/write guest data.
                if mbi.State==0x1000 and (mbi.Protect & 0xff) in (2,4,8) and not(mbi.Protect & 0x100) and 0x1000<=length<=0x40000000:
                    regions+=1
                    offset=0
                    while offset<length:
                        data=read(start+offset,min(0x400000+63,length-offset))
                        if data:
                            scanned+=len(data)
                            if a.locate_iop_toc_dma:
                                for item in find_iop_toc_dma_descriptors(data,start+offset):
                                    hits+=1
                                    print(f'active GetToc DMA descriptor host 0x{item["host_address"]:x}; guest MADR 0x{item["madr"]:x}',flush=True)
                                offset+=0x400000
                                continue
                            needle=anchor if anchor is not None else signatures[0][1]
                            index=data.find(needle)
                            while index>=0:
                                hits+=1
                                hit=start+offset+index
                                if anchor is not None:
                                    print(f'IOP anchor hit host 0x{hit:x} in region 0x{start:x}+0x{length:x}; guest module offset 0x{anchor_address:x}',flush=True)
                                else:
                                    candidate=hit-elf.entry
                                    print(f'entry candidate 0x{candidate:x}; probe matches '+str([read(candidate+pa,len(ps))==ps for pa,ps in signatures]),flush=True)
                                    if candidate not in found and verified(candidate):
                                        found.add(candidate); print(f'verified guest RAM candidate: 0x{candidate:x}',flush=True)
                                index=data.find(needle,index+1)
                        offset+=0x400000
                if start+length<=address: break
                address=start+length
            label='GetToc DMA descriptors' if a.locate_iop_toc_dma else ('IOP anchors' if anchor is not None else 'entry fingerprints')
            print(f'scanned {regions} non-executable regions, {scanned} bytes; {hits} {label}')
            if not found and anchor is None: print('large mappings (address, size, protection, type):',large)
            if anchor is None and not a.locate_iop_toc_dma and not found:
                raise ValueError('no RAM mapping found; ensure the game is loaded and paused')
        else:
            out=a.out.resolve(); repo=Path(__file__).resolve().parent.parent
            if out==repo or repo in out.parents: raise ValueError('oracle dumps must stay outside repository')
            if a.iop_base is not None:
                anchor_address,anchor=iop_anchor(a.iop_module)
                size=2*1024*1024
                def capture_verified():
                    data=read(a.iop_base,size)
                    if data is None: return None
                    offset=data.find(anchor)
                    if offset<0: return None
                    return data,offset
                if a.monitor_iop_gettoc_request_ms is not None:
                    deadline=time.perf_counter()+a.monitor_iop_gettoc_request_ms/1000
                    hits=[];weak_hits={};states={};samples=0;module_anchor_offset=None;madr=None;module_bases={}
                    initial=None
                    while time.perf_counter()<deadline and initial is None:
                        initial=capture_verified()
                    if initial is None:raise ValueError('IOP module anchor did not appear during GetToc request monitor')
                    _,module_anchor_offset=initial
                    module_base=module_anchor_offset-anchor_address
                    while time.perf_counter()<deadline:
                        data=read(a.iop_base,size)
                        if data is None:continue
                        current_anchor=data.find(anchor)
                        if current_anchor<0:continue
                        module_anchor_offset=current_anchor
                        module_base=module_anchor_offset-anchor_address
                        module_bases[module_base]=module_bases.get(module_base,0)+1
                        candidates=find_iop_cdvd_request_frames(data)
                        for candidate in candidates:
                            key=(candidate['descriptor'],candidate['madr'],candidate['saved_return'])
                            weak_hits[key]=weak_hits.get(key,0)+1
                        frames=[frame for frame in candidates
                                if frame['saved_return']==module_base+CDVDMAN_GETTOC_RETURN_OFFSET]
                        samples+=1
                        if frames:
                            if len(frames)!=1:raise ValueError('multiple original GetToc request frames observed')
                            frame=frames[0]
                            if madr is not None and madr!=frame['madr']:
                                raise ValueError('GetToc request MADR changed during one monitor window')
                            madr=frame['madr'];hits.append(frame)
                        if madr is not None:
                            toc=read(a.iop_base+madr,IOP_TOC_DMA_SIZE)
                            if toc is None:raise ValueError('unable to read observed GetToc DMA destination')
                            digest=hashlib.sha256(toc).hexdigest()
                            if digest in states:states[digest][2]+=1
                            else:
                                if len(states)>=256:raise ValueError('GetToc request monitor exceeded unique-state limit')
                                states[digest]=[toc,time.time_ns(),1]
                    out=a.out.resolve();repo=Path(__file__).resolve().parent.parent
                    if out==repo or repo in out.parents:raise ValueError('oracle dumps must stay outside repository')
                    out.mkdir(parents=True,exist_ok=True)
                    stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
                    captured=[]
                    for index,(digest,(toc,first,count)) in enumerate(states.items()):
                        target=out/f'toc-request-{stamp}-{index:03d}.bin';target.write_bytes(toc)
                        captured.append({'index':index,'sha256':digest,'status':toc_candidate_status(digest),
                                         'first_time_ns':first,'sample_count':count,'file':str(target)})
                    weak=[{'descriptor':descriptor,'madr':candidate_madr,'saved_return':saved_return,'sample_count':count}
                          for (descriptor,candidate_madr,saved_return),count in weak_hits.items()]
                    metadata={'utc':stamp,'pid':a.pid,'process':name.value,'host_base':hex(a.iop_base),
                              'duration_ms':a.monitor_iop_gettoc_request_ms,'map_samples':samples,
                              'request_frame_hits':len(hits),'first_request_frame':hits[0] if hits else None,
                              'observed_cdvd_frames':weak,
                              'observed_module_bases':[{'base':base,'sample_count':count}
                                                       for base,count in module_bases.items()],
                              'guest_madr':hex(madr) if madr is not None else None,'states':captured,'iop_module':a.iop_module,
                              'iop_anchor_module_offset':hex(anchor_address),
                              'iop_anchor_guest_offset':hex(module_anchor_offset),
                              'iop_module_guest_base':hex(module_anchor_offset-anchor_address),
                              'provenance':'original decoded GetToc wrapper return address plus exact DMA stack descriptor',
                              'access':'read-only; no process or guest writes','note':a.note}
                    report=out/f'toc-request-{stamp}.json';report.write_text(json.dumps(metadata,indent=2)+'\n')
                    print(json.dumps(metadata,indent=2))
                    if madr is None:raise ValueError('no exact original CDVDMAN GetToc request frame observed during bounded monitor')
                    return
                if a.monitor_iop_toc_ms is not None:
                    deadline=time.perf_counter()+a.monitor_iop_toc_ms/1000
                    initial=None
                    while time.perf_counter()<deadline and initial is None:
                        initial=capture_verified()
                    if initial is None:raise ValueError('IOP module anchor did not appear during monitor window')
                    _,module_anchor_offset=initial
                    toc_guest_offset=a.iop_toc_offset
                    unique={};samples=0;confirmed_samples=0;previous_toc=None
                    while time.perf_counter()<deadline:
                        toc=read(a.iop_base+toc_guest_offset,IOP_TOC_DMA_SIZE)
                        if toc is None:raise ValueError('unable to read monitored IOP TOC range')
                        samples+=1
                        if toc!=previous_toc:
                            previous_toc=toc;continue
                        confirmed_samples+=1;digest=hashlib.sha256(toc).hexdigest();now=time.time_ns()
                        if digest in unique:
                            unique[digest][2]=now;unique[digest][3]+=1
                        else:
                            if len(unique)>=256:raise ValueError('IOP TOC monitor exceeded unique-state limit')
                            unique[digest]=[toc,now,now,1]
                    final=capture_verified()
                    anchor_stable=final is not None and final[1]==module_anchor_offset
                    out=a.out.resolve();repo=Path(__file__).resolve().parent.parent
                    if out==repo or repo in out.parents:raise ValueError('oracle dumps must stay outside repository')
                    out.mkdir(parents=True,exist_ok=True)
                    stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
                    states=[]
                    for index,(digest,(toc,first,last,count)) in enumerate(unique.items()):
                        target=out/f'toc-monitor-{stamp}-{index:03d}.bin';target.write_bytes(toc)
                        states.append({'index':index,'sha256':digest,'status':toc_candidate_status(digest),
                                       'first_time_ns':first,'last_time_ns':last,'sample_count':count,
                                       'file':str(target)})
                    metadata={'utc':stamp,'pid':a.pid,'process':name.value,'host_base':hex(a.iop_base),
                              'duration_ms':a.monitor_iop_toc_ms,'samples':samples,'unique_states':states,
                              'confirmed_consecutive_samples':confirmed_samples,
                              'read_consistency':'states require two consecutive identical ReadProcessMemory results',
                              'iop_module':a.iop_module,'iop_anchor_module_offset':hex(anchor_address),
                              'iop_anchor_guest_offset':hex(module_anchor_offset),
                              'iop_module_guest_base':hex(module_anchor_offset-anchor_address),
                              'iop_anchor_stable_at_end':anchor_stable,
                              'guest_offset':hex(toc_guest_offset),'size':IOP_TOC_DMA_SIZE,
                              'note':a.note}
                    report=out/f'toc-monitor-{stamp}.json';report.write_text(json.dumps(metadata,indent=2)+'\n')
                    print(json.dumps(metadata,indent=2));return
                initial=capture_verified()
                if initial is None: raise ValueError('IOP base failed supplied-module anchor verification')
                data,module_anchor_offset=initial
                host_base=a.iop_base
                capture_kind='iop-ram'
            else:
                if not verified(a.base): raise ValueError('RAM base failed game-byte fingerprints')
                size=32*1024*1024
                data=read(a.base,size)
                module_anchor_offset=None
                host_base=a.base
                capture_kind='ee-ram'
            if data is None: raise ValueError('unable to read complete guest RAM')
            # Recheck the selected immutable identity after capture.  When not
            # using --suspend this detects, but cannot eliminate, a moving target.
            if a.iop_base is not None:
                final=capture_verified()
                if final is None or final[0]!=data: raise ValueError('IOP mapping changed during capture')
            elif not verified(a.base): raise ValueError('guest mapping changed during capture')
            out.mkdir(parents=True,exist_ok=True)
            stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
            target=out/f'{capture_kind}-{stamp}.bin'; target.write_bytes(data)
            metadata={'utc':stamp,'pid':a.pid,'process':name.value,'host_base':hex(host_base),
                      'guest_start':0,'size':len(data),'elf_sha256':elf.sha256,
                      'capture_sha256':hashlib.sha256(data).hexdigest(),'note':a.note,'file':str(target)}
            if module_anchor_offset is not None:
                metadata.update({'iop_module':a.iop_module,'iop_anchor_module_offset':hex(anchor_address),
                                 'iop_anchor_guest_offset':hex(module_anchor_offset),
                                 'iop_module_guest_base':hex(module_anchor_offset-anchor_address)})
            target.with_suffix('.json').write_text(json.dumps(metadata,indent=2)+'\n')
            print(json.dumps(metadata,indent=2))
            if a.iop_toc_candidate:
                # The address must come from the same original GetToc command
                # stage. Module anchors verify the RAM map but do not relocate
                # this caller-provided DMA destination.
                toc_offset=a.iop_toc_offset
                toc=extract_iop_toc_candidate(data,toc_offset)
                toc_target=out/f'toc-candidate-{stamp}.bin';toc_target.write_bytes(toc)
                toc_hash=hashlib.sha256(toc).hexdigest()
                toc_metadata={'utc':stamp,'source_capture':str(target),'source_capture_sha256':metadata['capture_sha256'],
                              'guest_offset':hex(toc_offset),'size':len(toc),'sha256':toc_hash,
                              'status':toc_candidate_status(toc_hash),
                              'note':a.note,'file':str(toc_target)}
                toc_target.with_suffix('.json').write_text(json.dumps(toc_metadata,indent=2)+'\n')
                print(json.dumps(toc_metadata,indent=2))
    finally:
        if suspended:
            status=ntdll.NtResumeProcess(handle)
            if status!=0: print(f'warning: NtResumeProcess failed ({status}); resume PCSX2 manually',file=sys.stderr)
        kernel.CloseHandle(handle)


if __name__=='__main__':
    try: main()
    except (ValueError,OSError) as e:
        print(f'error: {e}',file=sys.stderr); raise SystemExit(1)
