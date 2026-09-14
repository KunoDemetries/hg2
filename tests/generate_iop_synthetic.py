from pathlib import Path
import sys
from hgtool.iop_decode import decode
from hgtool.iop_emit import emit


def imm(op,rs,rt,value):return op<<26|rs<<21|rt<<16|(value&65535)
def reg(rs,rt,rd,fn):return rs<<21|rt<<16|rd<<11|fn

words={
    0x1000:imm(35,4,2,0),0x1004:imm(9,0,3,7),0x1008:reg(2,3,5,33),
    0x100c:reg(31,0,0,8),0x1010:imm(9,0,6,9),
    0x2000:imm(35,4,2,0),0x2004:reg(2,0,3,33),
    0x3000:2<<26|0x3010>>2,0x3004:imm(35,4,2,0),
    0x3010:imm(9,0,3,1),0x3014:reg(2,3,5,33),
    0x4000:imm(35,4,2,0),0x4004:imm(4,0,0,2),0x4008:reg(2,0,3,33),
    0x5000:reg(4,0,4,9),0x5004:reg(4,0,3,33),
    0x8000:reg(4,5,0,24),0x8004:reg(4,5,0,25),
    0x8008:reg(4,5,0,26),0x800c:reg(4,5,0,27),
    0x9000:imm(5,5,0,2),0x9004:reg(4,5,0,26),
    0xa000:imm(8,4,2,-1),0xa004:reg(4,5,2,32),0xa008:reg(4,5,2,34),
    0xb000:reg(31,0,0,8),0xb004:reg(4,5,0,32),
    0xf000:imm(34,4,8,3),0xf004:imm(38,4,8,0),0xf008:0,
    0xf00c:imm(42,5,8,3),0xf010:imm(46,5,8,0),0xf014:0,
    0xd000:0x40027800,0xd004:0,0xd008:0x00401821,
    0xe024:imm(9,29,29,-80),
}
Path(sys.argv[1]).write_text(emit({pc:decode(pc,w) for pc,w in words.items()},
                               {0x7000:'unresolved synthetic IOP import',0xc000:'unlinked import'},
                               {0xc000:(0x1000,0x2400000c)},
                               {0xe000:'flush_instruction_cache',0xe004:'flush_data_cache',
                                0xe008:'suspend_interrupts',0xe00c:'resume_interrupts',
                                0xe010:'register_interrupt',0xe014:'release_interrupt',0xe018:'enable_interrupt',
                                0xe01c:'disable_cpu_interrupts',0xe020:'exit_thread',
                                0x11000:'wait_vblank_start'},
                               {0xe024:'prepare_buffered_module'}),encoding='utf-8')
