#include "hg/controller.hpp"
namespace hg {
std::vector<std::uint8_t> Controller::exchange(const std::vector<std::uint8_t>& in,std::uint16_t buttons) {
        if(in.size()<5 || in[0]!=1 || in[2]!=0)
            throw std::runtime_error("invalid controller packet header");
        const auto command=in[1];
        if(!configuration && command!=0x42 && command!=0x43)
            throw std::runtime_error("controller command requires configuration mode");
        const auto length=configuration?9u:pressure?21u:analog?9u:5u;
        // A host may clock only the header/buttons to discover the new mode
        // before requesting its complete payload on the next poll.
        if(in.size()!=length && !(command==0x42 && in.size()<length && (in.size()==5 || in.size()==9)))
            throw std::runtime_error("unsupported controller packet length");
        std::vector<std::uint8_t> out(length,0);
        out[0]=0xff;out[1]=configuration?0xf3:pressure?0x79:analog?0x73:0x41;out[2]=0x5a;
        if(command==0x42 || (command==0x43 && !configuration)) {
            out[3]=std::uint8_t(buttons);out[4]=std::uint8_t(buttons>>8);
            if(length>=9)std::copy(axes.begin(),axes.end(),out.begin()+5);
            if(length==21 && command==0x42)std::copy(pressures.begin(),pressures.end(),out.begin()+9);
        }
        switch(command) {
        case 0x40:
            if(in[3]>=12 || in[4]!=2 || in[5]!=0 || in[6]!=0 || in[7]!=0 || in[8]!=0)
                throw std::runtime_error("unsupported controller pressure sensor parameters");
            initialized_pressure_sensors|=std::uint16_t(1u<<in[3]);
            out[5]=2;out[8]=0x5a;break;
        case 0x42:
            for(std::size_t n=0;n<std::min<std::size_t>(6,in.size()-3);++n) {
                if(rumble_map[n]==0)small_motor=in[n+3]&1;
                else if(rumble_map[n]==1)large_motor=in[n+3];
            }
            break;
        case 0x43:
            if(in[3]>1)throw std::runtime_error("unsupported controller configuration selector");
            configuration=in[3]!=0;break;
        case 0x44:
            if(in[3]>1)out[5]=0xff;
            else {analog=in[3]!=0;pressure=false;}
            analog_locked=(in[4]&3)==3;break;
        case 0x4f:
            if(in[3]!=0xff || in[4]!=0xff || in[5]!=3 || in[6]!=0 || in[7]!=0 || in[8]!=0)
                throw std::runtime_error("unsupported controller reply mask");
            analog=true;pressure=true;out[8]=0x5a;break;
        case 0x41:
            if(analog){out[3]=0xff;out[4]=0xff;out[5]=3;out[8]=0x5a;}
            break;
        case 0x45:
            out[3]=3;out[4]=2;out[5]=analog?1:0;out[6]=2;out[7]=1;break;
        case 0x46:
            if(in[3]<2){out[5]=1;out[6]=in[3]?1:2;out[7]=in[3]?1:0;out[8]=in[3]?20:10;}
            break;
        case 0x47:out[5]=2;out[7]=1;break;
        case 0x4c:out[6]=in[3]==0?4:in[3]==1?7:0;break;
        case 0x4d:
            for(unsigned n=0;n<6;++n)if(in[n+3]!=0xff && in[n+3]>1)
                throw std::runtime_error("unsupported controller actuator mapping");
            std::copy(rumble_map.begin(),rumble_map.end(),out.begin()+3);
            std::copy(in.begin()+3,in.end(),rumble_map.begin());break;
        default:throw std::runtime_error("unimplemented controller command "+std::to_string(command));
        }
        out.resize(in.size());
        return out;
    }
}
