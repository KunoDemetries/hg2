#include "hg/iop.hpp"

namespace hg {
void IopState::complete_sio2_card_probe() {
        auto& tx=dmac.channel[4];auto& rx=dmac.channel[5];
        // Bounded disconnected-card profile: one serial exchange, carried
        // in one 36-word DMA block in each direction by original MCMAN.
        const auto descriptor=sio2_registers[0];
        const auto input_length=(descriptor>>8)&0x1ffu,output_length=(descriptor>>18)&0x1ffu;
        if((descriptor&~((0x1ffu<<8)|(0x1ffu<<18)|1u))!=0x72u ||
           !input_length || input_length>144 || !output_length || output_length>144 ||
           std::any_of(sio2_registers.begin()+1,sio2_registers.begin()+16,[](auto v){return v!=0;}) ||
           !sio2_input_fifo.empty() || tx[2]!=0x01000201u || rx[2]!=0x01800200u ||
           tx[1]!=0x10024u || rx[1]!=0x10024u)
            throw IopFault(pc,"unsupported SIO2 DMA peripheral profile");
        constexpr unsigned bytes=144;
        for(const auto* c:{&tx,&rx})if((*c)[0]%4 || std::uint64_t((*c)[0])+bytes>ram.size())
            throw IopFault(pc,"SIO2 DMA buffer outside aligned RAM");
        if(ram[tx[0]]!=0x81)
            throw IopFault(pc,"unsupported SIO2 memory-card device selection");
        // No card drives the serial data line. Keep the DMA tail at the
        // same pulled-high value in this bounded profile (see SOURCES).
        sio2_input_fifo.assign(ram.begin()+tx[0],ram.begin()+tx[0]+input_length);
        for(unsigned n=0;n<bytes;n+=4)store(rx[0]+n,4,0xffffffffu);
        tx[0]+=bytes;rx[0]+=bytes;tx[1]&=0xffffu;rx[1]&=0xffffu;
        tx[2]&=~0x01000000u;rx[2]&=~0x01000000u;
        dmac.completed_channels|=(1u<<11)|(1u<<12);
        sio2_output_fifo.clear();sio2_output_cursor=0;
        sio2_registers[27]=0x1d100u;sio2_irq_pending=true;
    }

void IopState::complete_sio2_poll() {
        if(sio2_irq_pending || sio2_output_cursor<sio2_output_fifo.size())
            throw IopFault(pc,"SIO2 transfer overlaps an unconsumed reply");
        if(sio2_registers[0]&0x30u) {complete_sio2_card_probe();return;}
        // Bounded CPU-FIFO profile for pads and disconnected cards. Validate everything
        // before publishing response bytes or completion.
        const auto port=sio2_registers[0]&3u;
        const auto descriptor=sio2_registers[0];
        const auto input_length=(descriptor>>8)&0x1ffu,output_length=(descriptor>>18)&0x1ffu;
        if((descriptor&~((0x1ffu<<8)|(0x1ffu<<18)|3u))!=0x40u ||
           input_length!=sio2_input_fifo.size() || !input_length || input_length>256 || !output_length || output_length>256 ||
           std::any_of(sio2_registers.begin()+1,sio2_registers.begin()+16,[](auto v){return v!=0;}))
            throw IopFault(pc,"unsupported SIO2 peripheral command/profile");
        if(port>=2) {
            if(sio2_input_fifo[0]!=0x81)throw IopFault(pc,"unsupported SIO2 memory-card device selection");
            sio2_output_fifo.assign(output_length,0xff);sio2_output_cursor=0;
            sio2_registers[27]=0x1d100u;sio2_irq_pending=true;return;
        }
        if(input_length!=output_length || (input_length!=5 && input_length!=9 && input_length!=21))
            throw IopFault(pc,"unsupported SIO2 controller packet length");
        if(sio2_connected[port]) {
            // Transactional: rejected profiles must not partially change pad state.
            auto controller=sio2_controllers[port];
            try {sio2_output_fifo=controller.exchange(sio2_input_fifo,sio2_buttons[port]);}
            catch(const std::exception& e){throw IopFault(pc,e.what());}
            sio2_controllers[port]=controller;
        } else sio2_output_fifo.assign(output_length,0xff);
        sio2_output_cursor=0;
        sio2_registers[27]=sio2_connected[port]?0x1100u:0x1d100u;
        sio2_irq_pending=true;
    }

std::uint32_t IopState::load_sio2(std::uint32_t address,unsigned size) {
        if(address==0x1f808264 && size==1) {
            if(sio2_output_cursor>=sio2_output_fifo.size())throw IopFault(pc,"SIO2 output FIFO read before a completed peripheral transfer");
            return sio2_output_fifo[sio2_output_cursor++];
        }
        if(size!=4 || address%4)throw IopFault(pc,"invalid SIO2 register width/alignment");
        return sio2_registers[(address-0x1f808200)/4];
    }

void IopState::store_sio2(std::uint32_t address,unsigned size,std::uint32_t value) {
        if(address==0x1f808260 && size==1) {sio2_input_fifo.push_back(std::uint8_t(value));return;}
        if(size!=4 || address%4)throw IopFault(pc,"invalid SIO2 register width/alignment");
        auto& target=sio2_registers[(address-0x1f808200)/4];
        if(address==0x1f808268) {
            sio2_last_control_write=value;
            if(value&1u) {complete_sio2_poll();target=value&~1u;return;}
            if(value&0x0cu) {sio2_input_fifo.clear();sio2_output_fifo.clear();sio2_output_cursor=0;}
        }
        target=value;
    }
}
