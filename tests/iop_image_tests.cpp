#include "hg/iop_image.hpp"
#include "hg/image.hpp"
#include <iostream>
#define CHECK(x) do {if(!(x)){std::cerr<<"Failed line "<<__LINE__<<": "<<#x<<'\n';return 1;}}while(0)
int main() {
    hg::IopState s;std::fill(s.ram.begin(),s.ram.end(),0xcc);
    const std::vector<std::uint8_t> input{1,2,3,4,5,6,7,8};const auto hash=hg::sha256(input);
    bool fault=false;
    try{hg::load_iop_image(s,input,hash,{{0,0x1000,8,32}},{{0x1000,0,0x11223344}});}
    catch(const std::runtime_error&){fault=true;}
    CHECK(fault && s.ram[0x1000]==0xcc && s.ram[0x1010]==0xcc);
    hg::load_iop_image(s,input,hash,{{0,0x1000,8,32}},{{0x1000,0x04030201,0x11223344}});
    CHECK(s.load(0x1000,4,false)==0x11223344 && s.load(0x1004,4,false)==0x08070605);
    CHECK(s.ram[0x101f]==0 && s.ram[0x1020]==0xcc);
    fault=false;try{hg::load_iop_image(s,input,hash,{{0,0x200000,8,32}},{});}
    catch(const std::runtime_error&){fault=true;}CHECK(fault);
    fault=false;try{hg::load_iop_image(s,input,"bad",{{0,0x2000,8,32}},{});}
    catch(const std::runtime_error&){fault=true;}CHECK(fault && s.ram[0x2000]==0xcc);
    std::vector<std::uint8_t> container(16,0xaa);container.insert(container.end(),input.begin(),input.end());
    const auto container_hash=hg::sha256(container);
    hg::load_iop_region(s,container,container_hash,16,8,hash,{{0,0x3000,8,32}},{});
    CHECK(s.load(0x3000,4,false)==0x04030201 && s.ram[0x301f]==0);
    fault=false;try{hg::load_iop_region(s,container,container_hash,0xfffffff0,32,hash,{{0,0x4000,8,32}},{});}
    catch(const std::runtime_error&){fault=true;}CHECK(fault && s.ram[0x4000]==0xcc);
    container[0]^=1;
    fault=false;try{hg::load_iop_region(s,container,container_hash,16,8,hash,{{0,0x4000,8,32}},{});}
    catch(const std::runtime_error&){fault=true;}CHECK(fault && s.ram[0x4000]==0xcc);
    fault=false;try{hg::load_iop_region(s,container,hg::sha256(container),16,8,"bad",{{0,0x4000,8,32}},{});}
    catch(const std::runtime_error&){fault=true;}CHECK(fault && s.ram[0x4000]==0xcc);
}
