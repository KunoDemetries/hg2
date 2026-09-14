#include "hg/image.hpp"
#include <algorithm>
#include <iostream>
#include <fstream>

static bool dump_state(const hg::State& state,const std::filesystem::path& prefix) {
    if(prefix.empty()) return true;
    auto ram_path=prefix; ram_path+=".ram";
    auto metadata_path=prefix; metadata_path+=".json";
    std::ofstream ram(ram_path,std::ios::binary), metadata(metadata_path);
    if(!ram || !metadata) {std::cerr<<"Cannot create diagnostic state files\n"; return false;}
    ram.write(reinterpret_cast<const char*>(state.ram.data()),state.ram.size());
    metadata<<"{\n  \"image_sha256\": \""<<hg::compiled_image_sha256()<<"\",\n  \"pc\": "<<state.pc
        <<",\n  \"last_transfer_pc\": "<<state.last_transfer_pc
        <<",\n  \"current_thread\": "<<state.boot.current_thread<<",\n  \"gpr\": [\n";
    for(unsigned i=0;i<32;++i)
        metadata<<"    [\"0x"<<std::hex<<state.r(i)<<"\", \"0x"<<(i?state.gpr[i].hi:0)
            <<"\"]"<<(i==31?"\n":",\n");
    metadata<<"  ]\n}\n";
    ram.close(); metadata.close();
    if(!ram || !metadata) {std::cerr<<"Cannot finish diagnostic state files\n"; return false;}
    std::cout<<"Native state dumped to "<<prefix.string()<<".{ram,json}\n";
    return true;
}
int main(int argc, char** argv) {
    bool verify = false, boot = false;
    std::filesystem::path elf = "Haunting Ground (USA)/SLUS_210.75";
    std::filesystem::path dump;
    for (int i=1;i<argc;++i) {
        if (std::string(argv[i]) == "--verify-prefix") verify=true;
        else if (std::string(argv[i]) == "--boot") boot=true;
        else if (std::string(argv[i]) == "--elf" && i+1<argc) elf=argv[++i];
        else if (std::string(argv[i]) == "--dump-state" && i+1<argc) dump=argv[++i];
        else { std::cerr << "Usage: hg_diagnostic [--verify-prefix] [--boot] [--elf path] [--dump-state external-prefix]\n"; return 1; }
    }
    hg::State state;
    try { hg::load_elf_file(state,elf,hg::compiled_image_sha256()); }
    catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
    state.protect_kernel_memory=true;
    hg::configure_compiled_image(state);
    for (auto& r : state.gpr) r = {0x12345678, 0xabcdef01};
    state.hi = state.lo = state.hi1 = state.lo1 = 123;
    state.sa = 123;
    state.fpr.fill(0x12345678);
    try {
        hg::run(state, 66);
        while(state.pump_vif1()) {}
        while(state.pump_gif()) {}
        bool reset=state.pc==0x00100110;
        for (unsigned i=1;i<32;++i) {
            if (i==26 || i==27) reset=reset && state.gpr[i].lo==0x12345678 && state.gpr[i].hi==0xabcdef01;
            else reset=reset && state.gpr[i].lo==0 && state.gpr[i].hi==0;
        }
        for (auto bits:state.fpr) reset=reset && bits==0;
        reset=reset && state.hi==0 && state.lo==0 && state.hi1==0 && state.lo1==0 && state.sa==0;
        if (!reset) { std::cerr << "Startup register reset failed\n"; return 1; }
        std::cout << "Startup register reset verified (including preserved k0/k1)\n";
        // Poison the independently decoded startup clear range so a no-op clear
        // cannot pass merely because the loader already zeroed the BSS.
        std::fill(state.ram.begin()+0x47b200,state.ram.begin()+0x1992000,0xa5);
        state.ram[0x47b1ff]=0x5a;
        hg::run(state, 20000000);
        while(state.pump_vif1()) {}
        while(state.pump_gif()) {}
        std::cerr << "Budget reached without expected diagnostic trap\n";
        return 1;
    } catch (const hg::Fault& fault) {
        std::cout << "Translation stopped at 0x" << std::hex << fault.pc
                  << ": " << fault.what() << '\n';
        const bool cleared=std::all_of(state.ram.begin()+0x47b200,state.ram.begin()+0x1992000,[](auto b){return b==0;});
        const bool expected=fault.pc==0x001001c8 && cleared && state.ram[0x47b1ff]==0x5a
            && state.fpu.acc==0 && state.fpu.control==0x01000001 && state.r(3)==0x3c
            && state.r(4)==0x4828f0 && state.r(5)==~0ull && state.r(6)==0x8000
            && state.r(7)==0x19709c0 && state.r(8)==0x100220;
        if (!expected) { std::cerr << "Startup BSS/kernel boundary verification failed\n"; return 1; }
        std::cout << "BSS clear and first kernel-call arguments verified\n";
        if(boot) {
            state.boot.enabled=true;
            try { hg::run(state,20000000); while(state.pump_vif1()) {} while(state.pump_gif()) {} }
            catch(const hg::Fault& next) {
                std::cout << "Native boot stopped at 0x" << std::hex << next.pc << ": " << next.what()
                          << "; sp=0x" << state.r(29) << "; a0=0x" << state.r(4)
                          << "; a1=0x" << state.r(5) << "; a2=0x" << state.r(6)
                          << "; ra=0x" << state.r(31) << '\n';
                return dump_state(state,dump) ? 2:1;
            }
            std::cout << "Native boot dispatch budget reached\n";
            return dump_state(state,dump) ? 2:1;
        }
        return verify ? 0 : 2;
    }
}
