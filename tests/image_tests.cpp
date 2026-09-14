#include "hg/image.hpp"
#include <iostream>
#define CHECK(x) do { if (!(x)) { std::cerr << "Failed line " << __LINE__ << ": " << #x << '\n'; return 1; } } while (0)
int main() {
    CHECK(hg::sha256({}) == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    CHECK(hg::sha256({'a','b','c'}) == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    CHECK(hg::sha256(std::vector<std::uint8_t>(1000000,'a')) == "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
    std::vector<std::uint8_t> bytes(88);
    const auto put=[&](unsigned off,std::uint32_t v,unsigned size=4) {
        for(unsigned i=0;i<size;++i) bytes.at(off+i)=std::uint8_t(v>>(i*8));
    };
    put(0,0x464c457f); put(4,1,1); put(5,1,1); put(6,1,1);
    put(16,2,2); put(18,8,2); put(20,1); put(24,0x1000); put(28,52);
    put(40,52,2); put(42,32,2); put(44,1,2);
    put(52,1); put(56,84); put(60,0x1000); put(68,4); put(72,16); put(76,5);
    put(84,0x12345678);
    hg::State s; std::fill(s.ram.begin(),s.ram.end(),0xcc);
    CHECK(hg::load_elf(s,bytes,hg::sha256(bytes))==0x1000);
    CHECK(s.load(0x1000,4,false)==0x12345678 && s.load(0x1004,4,false)==0);
    CHECK(s.ram[0xfff]==0xcc && s.ram[0x1010]==0xcc);
    bool failed=false;
    try { hg::load_elf(s,bytes,"wrong"); } catch(const std::runtime_error&) { failed=true; }
    CHECK(failed);
    put(72,0xffffffff);
    failed=false;
    try { hg::load_elf(s,bytes,hg::sha256(bytes)); } catch(const std::runtime_error&) { failed=true; }
    CHECK(failed && s.load(0x1000,4,false)==0x12345678);
    return 0;
}
