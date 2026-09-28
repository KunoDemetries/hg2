#include "hg/iop_image.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#define CHECK(x) do {if(!(x)){std::cerr<<"Failed line "<<__LINE__<<": "<<#x<<'\n';return 1;}}while(0)
int main(int argc,char** argv) {
    CHECK(argc==2);const std::filesystem::path inputs=argv[1];
    hg::IopState s;s.w(4,0x12345678);
    hg::load_compiled_iop(s,inputs);
    CHECK(hg::compiled_iop_high_water_mark()==0x00030100u);
    CHECK(s.static_iop_module_plans.size()==1);
    const auto& plan=s.static_iop_module_plans[0];
    CHECK(plan.base==0x30000 && plan.memory_size==0x100 && plan.allocation_prefix==0x30 && !plan.allocated);
    CHECK(!plan.buffer_image.empty() && plan.buffer_image[0]==0x7f);
    std::copy(plan.buffer_image.begin(),plan.buffer_image.end(),s.ram.begin()+0x40000);
    s.prepare_buffered_module(0x40000,0,0);
    CHECK(s.alloc_system_memory(0,0x130,0)==0x2ffd0);
    for(const auto& item:{std::pair<const char*,unsigned>{"a",11},{"b",22},{"c",33}}) {
        s.pc=hg::compiled_iop_module_export(item.first,"exports",0);s.w(31,0x1f0000);
        hg::run_iop(s,1);CHECK(s.pc==0x1f0000 && s.r(2)==item.second && s.r(4)==0x12345678);
    }
    bool fault=false;try{hg::compiled_iop_entry();}catch(const std::runtime_error&){fault=true;}CHECK(fault);
    // The first valid module must not change the destination if the second fails.
    const auto temp=std::filesystem::temp_directory_path()/
        ("hg-iop-bundle-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    CHECK(std::filesystem::create_directory(temp));
    std::filesystem::copy_file(inputs/"a.irx",temp/"a.irx");
    {std::ofstream f(temp/"b.img",std::ios::binary);f<<"invalid container";}
    const auto before=s.ram;s.pc=0x7654;fault=false;
    try{hg::load_compiled_iop(s,temp);}catch(const std::runtime_error&){fault=true;}
    std::filesystem::remove(temp/"a.irx");std::filesystem::remove(temp/"b.img");std::filesystem::remove(temp);
    CHECK(fault && s.ram==before && s.pc==0x7654 && s.r(4)==0x12345678);
}
