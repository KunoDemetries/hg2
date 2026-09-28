#include "hg/iop_image.hpp"
#include <iostream>

static std::uint32_t invoke_address(hg::IopState& s,std::uint32_t address,std::uint32_t argument,
                                    std::uint32_t second=0,std::uint32_t third=0) {
    s.gpr.fill(0);s.pending_reg=s.next_reg=0;
    s.pc=address;
    s.w(4,argument);s.w(5,second);s.w(6,third);s.w(29,0x1ef000);s.w(31,0x1f0000);
    for(unsigned steps=0;steps<10000 && s.pc!=0x1f0000;++steps)hg::run_iop(s,1);
    if(s.pc!=0x1f0000)throw hg::IopFault(s.pc,"IOP call reached its diagnostic instruction budget");
    return s.r(2);
}
static std::uint32_t invoke(hg::IopState& s,unsigned ordinal,std::uint32_t argument,
                            std::uint32_t second=0,std::uint32_t third=0,const char* library=nullptr,const char* module=nullptr) {
    const auto address=module?hg::compiled_iop_module_export(module,library,ordinal):library?hg::compiled_iop_library_export(library,ordinal):hg::compiled_iop_export(ordinal);
    return invoke_address(s,address,argument,second,third);
}
int main(int argc,char** argv) {
    std::filesystem::path path="Haunting Ground (USA)/MODULES/MODMSIN.IRX";
    bool verify=false,sysclib=false,bootmodes=false,bundle=false,start_sif=false;
    for(int n=1;n<argc;++n) {
        const std::string arg=argv[n];
        if(arg=="--verify-modmsin")verify=true;
        else if(arg=="--verify-sysclib")sysclib=true;
        else if(arg=="--verify-bootmodes")bootmodes=true;
        else if(arg=="--verify-bundle")bundle=true;
        else if(arg=="--start-sif")start_sif=true;
        else if(arg=="--module" && n+1<argc)path=argv[++n];
        else {std::cerr<<"Usage: hg_iop_diagnostic [--verify-modmsin|--verify-sysclib|--verify-bootmodes|--verify-bundle|--start-sif] [--module path]\n";return 1;}
    }
    hg::IopState s;
    try {
        hg::load_compiled_iop(s,path);
        if(unsigned(verify)+unsigned(sysclib)+unsigned(bootmodes)+unsigned(bundle)+unsigned(start_sif)>1)throw std::runtime_error("choose one IOP verification mode");
        if(start_sif) {
            if(std::string(hg::compiled_iop_name())!="bundle")throw std::runtime_error("SIF startup needs a combined build");
            // Native diagnostic profile starts with no boot-mode records. Execute
            // the original pointer setup; do not fabricate SIF-ready state.
            const auto loadcore=hg::compiled_iop_module_entry("rom_loadcore");
            s.pc=loadcore+0xcc;hg::run_iop(s,4);
            for(const auto& pair:{std::pair<const char*,const char*>{"rom_sifman","loadcore"},
                                  {"rom_sifcmd","loadcore"},{"rom_sifcmd","sifman"}}) {
                const auto provider=std::string(pair.second)=="loadcore"?"rom_loadcore":"rom_sifman";
                invoke_address(s,loadcore+0x11d8,hg::compiled_iop_table(pair.first,pair.second,true),
                               hg::compiled_iop_table(provider,pair.second,false));
            }
            std::cout<<"Starting original SIFMAN entry\n";
            invoke_address(s,hg::compiled_iop_module_entry("rom_sifman"),0);
            std::cout<<"Starting original SIFCMD entry\n";
            invoke_address(s,hg::compiled_iop_module_entry("rom_sifcmd"),0);
            std::cout<<"SIF module entry routines returned\n";return 0;
        }
        if(bundle) {
            if(std::string(hg::compiled_iop_name())!="bundle")throw std::runtime_error("bundle verification needs combined modules");
            s.store(0x2000,4,0x12345678);
            if(invoke(s,12,0x2100,0x2000,4,"sysclib","rom_sysclib")!=0x2100 || s.load(0x2100,4,false)!=0x12345678)
                throw std::runtime_error("combined SYSCLIB copy failed");
            // MODMSIN's sole export library name comes from its supplied table.
            if(invoke(s,4,0,0,0,"modmsin","modmsin")!=0xffffffffu)throw std::runtime_error("combined MODMSIN validator failed");
            s.pc=hg::compiled_iop_module_entry("rom_loadcore")+0xcc;hg::run_iop(s,4);
            s.store(0x2000,4,0x00074242);
            invoke(s,13,0x2000,0,0,"loadcore","rom_loadcore");
            if(invoke(s,12,7,0,0,"loadcore","rom_loadcore")!=s.load(0x3f0,4,false))
                throw std::runtime_error("combined LOADCORE boot-mode lookup failed");
            if(s.load(0x2100,4,false)!=0x12345678)throw std::runtime_error("combined module memory was not preserved");
            const auto imports=hg::compiled_iop_table("rom_sifcmd","loadcore",true);
            const auto exports=hg::compiled_iop_table("rom_loadcore","loadcore",false);
            // SIFCMD's second loadcore stub is ordinal12, established from its import table.
            const auto query_stub=imports+28;bool blocked=false;
            try{invoke_address(s,query_stub,7);}catch(const hg::IopFault&){blocked=true;}
            if(!blocked)throw std::runtime_error("unlinked SIFCMD import executed");
            invoke_address(s,hg::compiled_iop_module_entry("rom_loadcore")+0x11d8,imports,exports);
            if(invoke_address(s,query_stub,7)!=s.load(0x3f0,4,false))throw std::runtime_error("linked SIFCMD boot-mode query failed");
            // Test minor-version compatibility using the original supplied
            // LOADCORE linker itself. Reset the import table/stub, lower only
            // its minor version, then inspect whether LOADCORE patches it.
            const auto import_version=s.load(imports+8,2,false);
            const auto export_version=s.load(exports+8,2,false);
            if(import_version!=export_version || !(import_version&0xff))
                throw std::runtime_error("LOADCORE compatibility fixture needs equal nonzero minor versions");
            s.store(imports+8,2,import_version-1);
            s.store(query_stub,4,0x03e00008);
            invoke_address(s,hg::compiled_iop_module_entry("rom_loadcore")+0x11d8,imports,exports);
            const auto query_target=hg::compiled_iop_module_export("rom_loadcore","loadcore",12);
            const auto expected_jump=0x08000000u|((query_target>>2)&0x03ffffffu);
            if(s.load(query_stub,4,false)!=expected_jump)
                throw std::runtime_error("original LOADCORE rejected older-minor import compatibility");
            std::cout<<"Native IOP bundle verified; original LOADCORE also linked an older-minor import to its newer export\n";return 0;
        }
        if(bootmodes) {
            if(std::string(hg::compiled_iop_name())!="rom_loadcore")throw std::runtime_error("boot-mode verification requires LOADCORE");
            // The supplied module's four-word initialization fragment sets both
            // low-memory boot-table pointers. This is an isolated fixture, not full boot.
            s.pc=hg::compiled_iop_entry()+0xcc;hg::run_iop(s,4);
            const auto table=s.load(0x3f0,4,false);
            if(!table || table!=s.load(0x3f4,4,false))throw std::runtime_error("boot-table initialization mismatch");
            if(invoke(s,12,7,0,0,"loadcore")!=0)throw std::runtime_error("empty boot table returned a mode");
            s.store(0x2000,4,0x01071234);s.store(0x2004,4,0xa1b2c3d4);
            invoke(s,13,0x2000,0,0,"loadcore");
            if(invoke(s,12,7,0,0,"loadcore")!=table || s.load(table+4,4,false)!=0xa1b2c3d4)
                throw std::runtime_error("registered boot mode was not preserved");
            s.store(0x2000,4,0x00094242);invoke(s,13,0x2000,0,0,"loadcore");
            if(invoke(s,12,9,0,0,"loadcore")!=table+8 || invoke(s,12,8,0,0,"loadcore")!=0)
                throw std::runtime_error("boot-mode record traversal mismatch");
            // The game's bounded register routine ignores a record larger than its table.
            const auto end=s.load(0x3f4,4,false);s.store(0x2000,4,0xff0a1111);
            invoke(s,13,0x2000,0,0,"loadcore");
            if(s.load(0x3f4,4,false)!=end || invoke(s,12,10,0,0,"loadcore")!=0)
                throw std::runtime_error("oversized boot mode changed the table");
            std::cout<<"LOADCORE boot-mode registration/query executed as native AOT code: five cases passed\n";return 0;
        }
        if(sysclib) {
            if(verify || std::string(hg::compiled_iop_name())!="rom_sysclib")throw std::runtime_error("SYSCLIB verification needs the matching compiled module");
            unsigned cases=0;
            for(unsigned length: {0u,1u,3u,4u,7u,16u,31u,64u})for(unsigned alignment=0;alignment<4;++alignment) {
                const auto src=0x2000+alignment,dst=0x3000+((alignment+1)%4);
                std::fill(s.ram.begin()+0x1ff0,s.ram.begin()+0x3100,0xcc);
                for(unsigned n=0;n<length;++n)s.store(src+n,1,(n*73+19)&255);
                if(invoke(s,12,dst,src,length,"sysclib")!=dst)throw std::runtime_error("SYSCLIB memcpy return mismatch");
                for(unsigned n=0;n<length;++n)if(s.load(dst+n,1,false)!=((n*73+19)&255))throw std::runtime_error("SYSCLIB memcpy data mismatch");
                if(s.load(dst-1,1,false)!=0xcc || s.load(dst+length,1,false)!=0xcc)throw std::runtime_error("SYSCLIB memcpy overrun");
                if(invoke(s,14,dst,0x15a,length,"sysclib")!=dst)throw std::runtime_error("SYSCLIB memset return mismatch");
                for(unsigned n=0;n<length;++n)if(s.load(dst+n,1,false)!=0x5a)throw std::runtime_error("SYSCLIB memset data mismatch");
                if(s.load(dst-1,1,false)!=0xcc || s.load(dst+length,1,false)!=0xcc)throw std::runtime_error("SYSCLIB memset overrun");
                for(unsigned n=0;n<length;++n)s.store(src+n,1,'a'+n%26);
                s.store(src+length,1,0);
                if(invoke(s,27,src,0,0,"sysclib")!=length)throw std::runtime_error("SYSCLIB strlen result mismatch");
                cases+=3;
            }
            struct FormatCase {const char* format;std::uint32_t argument;const char* expected;};
            for(const auto& test: {FormatCase{"hello",0,"hello"},FormatCase{"%d",0xffffff85,"-123"},
                                  FormatCase{"%08x",0x12ab,"000012ab"},FormatCase{"%u",0xffffffff,"4294967295"},
                                  FormatCase{"[%s]",0x2200,"[native]"}}) {
                const std::string format=test.format,expected=test.expected,value="native";
                std::fill(s.ram.begin()+0x3000,s.ram.begin()+0x3100,0xcc);
                for(unsigned n=0;n<=format.size();++n)s.store(0x2100+n,1,test.format[n]);
                for(unsigned n=0;n<=value.size();++n)s.store(0x2200+n,1,value.c_str()[n]);
                if(invoke(s,19,0x3000,0x2100,test.argument,"sysclib")!=expected.size())throw std::runtime_error("SYSCLIB sprintf length mismatch");
                for(unsigned n=0;n<=expected.size();++n)if(s.load(0x3000+n,1,false)!=std::uint8_t(expected.c_str()[n]))
                    throw std::runtime_error("SYSCLIB sprintf output mismatch");
                if(s.load(0x3001+unsigned(expected.size()),1,false)!=0xcc)throw std::runtime_error("SYSCLIB sprintf overrun");
                ++cases;
            }
            std::cout<<"SYSCLIB memory/string/formatting executed as native AOT code: "<<cases<<" cases passed\n";return 0;
        }
        if(verify) {
            if(std::string(hg::compiled_iop_name())!="modmsin")throw std::runtime_error("MODMSIN verification needs the matching compiled module");
            // Independently decoded export 4 validates a descriptor and its entries.
            if(invoke(s,4,0)!=0xffffffffu)throw std::runtime_error("null descriptor was not rejected");
            s.store(0x2000,4,2);s.store(0x2004,4,0x2100);
            s.store(0x2108,4,1);s.store(0x210c,4,0x2200);
            s.store(0x2204,4,0x2300);s.store(0x2300,4,8);
            if(invoke(s,4,0x2000)!=0)throw std::runtime_error("valid descriptor was rejected");
            s.store(0x2000,4,1);
            if(invoke(s,4,0x2000)!=0xffffffffu)throw std::runtime_error("invalid descriptor tag was not rejected");
            s.store(0x2000,4,2);
            s.store(0x2300,4,7);
            if(invoke(s,4,0x2000)!=0xffffffffu)throw std::runtime_error("short entry was not rejected");
            s.store(0x2204,4,0);
            if(invoke(s,4,0x2000)!=0xffffffffu)throw std::runtime_error("null entry was not rejected");
            std::cout<<"MODMSIN descriptor validation executed as native AOT code: five cases passed\n";
            return 0;
        }
        s.pc=hg::compiled_iop_entry();s.w(29,0x1ef000);s.w(31,0x1f0000);
        hg::run_iop(s,10000);
        std::cerr<<"IOP budget exhausted at 0x"<<std::hex<<s.pc<<'\n';return 2;
    } catch(const hg::IopFault& e) {
        std::cerr<<"Native IOP stopped at 0x"<<std::hex<<e.pc<<": "<<e.what()<<'\n';return 2;
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
