"""Conservative MIPS-I integer decoder for IOP modules, separate from EE rules."""
from .decode import Instruction


def decode(pc,word):
    op,rs,rt,rd,sa,fn=word>>26,(word>>21)&31,(word>>16)&31,(word>>11)&31,(word>>6)&31,word&63
    immediate=(word&65535)-(65536 if word&32768 else 0)
    name='unsupported';target=None
    if op==0:
        name={0:'sll',2:'srl',3:'sra',4:'sllv',6:'srlv',7:'srav',8:'jr',9:'jalr',
              12:'syscall',13:'break',16:'mfhi',17:'mthi',18:'mflo',19:'mtlo',
              24:'mult',25:'multu',26:'div',27:'divu',
              32:'add',33:'addu',34:'sub',35:'subu',36:'and',37:'or',38:'xor',39:'nor',42:'slt',43:'sltu'}.get(fn,name)
        if name in ('sll','srl','sra') and rs:name='unsupported'
        elif name=='jr' and (rt or rd or sa):name='unsupported'
        elif name=='jalr' and (rt or sa):name='unsupported'
        elif name in ('mfhi','mflo') and (rs or rt or sa):name='unsupported'
        elif name in ('mthi','mtlo') and (rt or rd or sa):name='unsupported'
        elif name in ('mult','multu','div','divu') and (rd or sa):name='unsupported'
        elif name in ('sllv','srlv','srav','add','sub','addu','subu','and','or','xor','nor','slt','sltu') and sa:name='unsupported'
    elif op in (2,3):
        name='j' if op==2 else 'jal';target=((pc+4)&0xf0000000)|((word&0x3ffffff)<<2)
    elif op in (4,5,6,7):
        name={4:'beq',5:'bne',6:'blez',7:'bgtz'}[op]
        target=(pc+4+immediate*4)&0xffffffff
        if op in (6,7) and rt:name='unsupported'
    elif op==1 and rt in (0,1):
        name='bltz' if rt==0 else 'bgez';target=(pc+4+immediate*4)&0xffffffff
    elif op==16 and rs in (0,4) and not(word&0x7ff):name='mfc0' if rs==0 else 'mtc0'
    else:
        name={8:'addi',9:'addiu',10:'slti',11:'sltiu',12:'andi',13:'ori',14:'xori',15:'lui',
              32:'lb',33:'lh',34:'lwl',35:'lw',36:'lbu',37:'lhu',38:'lwr',40:'sb',41:'sh',42:'swl',43:'sw',46:'swr'}.get(op,name)
        if op==15 and rs:name='unsupported'
    return Instruction(pc,word,name,rs,rt,rd,sa,immediate,target)
