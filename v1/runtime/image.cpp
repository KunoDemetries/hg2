#include "hg/image.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace hg {
std::string sha256(const std::vector<std::uint8_t>& input) {
    constexpr std::uint32_t k[] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
    std::array<std::uint32_t, 8> h{0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
                                  0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
    auto bytes = input;
    const auto bits = std::uint64_t(input.size()) * 8;
    bytes.push_back(0x80);
    while (bytes.size() % 64 != 56) bytes.push_back(0);
    for (int i = 7; i >= 0; --i) bytes.push_back(std::uint8_t(bits >> (i * 8)));
    const auto rot = [](std::uint32_t x, unsigned n) { return (x >> n) | (x << (32 - n)); };
    for (std::size_t offset = 0; offset < bytes.size(); offset += 64) {
        std::uint32_t w[64]{};
        for (unsigned i = 0; i < 16; ++i)
            for (unsigned j = 0; j < 4; ++j) w[i] = (w[i] << 8) | bytes[offset + i * 4 + j];
        for (unsigned i = 16; i < 64; ++i) {
            const auto s0 = rot(w[i-15],7) ^ rot(w[i-15],18) ^ (w[i-15] >> 3);
            const auto s1 = rot(w[i-2],17) ^ rot(w[i-2],19) ^ (w[i-2] >> 10);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }
        auto a=h[0], b=h[1], c=h[2], d=h[3], e=h[4], f=h[5], g=h[6], z=h[7];
        for (unsigned i = 0; i < 64; ++i) {
            const auto t1 = z + (rot(e,6)^rot(e,11)^rot(e,25)) + ((e&f)^(~e&g)) + k[i] + w[i];
            const auto t2 = (rot(a,2)^rot(a,13)^rot(a,22)) + ((a&b)^(a&c)^(b&c));
            z=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
        }
        h[0]+=a; h[1]+=b; h[2]+=c; h[3]+=d; h[4]+=e; h[5]+=f; h[6]+=g; h[7]+=z;
    }
    std::ostringstream out;
    for (auto value : h) out << std::hex << std::setfill('0') << std::setw(8) << value;
    return out.str();
}

std::uint32_t load_elf(State& state, const std::vector<std::uint8_t>& bytes, const std::string& expected_hash) {
    if (sha256(bytes) != expected_hash) throw std::runtime_error("ELF does not match statically compiled image SHA-256");
    const auto get = [&](std::size_t off, unsigned count) {
        if (off > bytes.size() || count > bytes.size() - off) throw std::runtime_error("truncated ELF");
        std::uint32_t v=0;
        for (unsigned i=0;i<count;++i) v |= std::uint32_t(bytes[off+i]) << (i*8);
        return v;
    };
    if (bytes.size() < 52 || get(0,4)!=0x464c457f || get(4,1)!=1 || get(5,1)!=1 || get(6,1)!=1
        || get(16,2)!=2 || get(18,2)!=8 || get(20,4)!=1 || get(40,2)!=52 || get(42,2)!=32)
        throw std::runtime_error("expected ELF32 little-endian MIPS executable");
    const auto entry=get(24,4), ph=get(28,4), count=get(44,2);
    if (std::uint64_t(ph) + std::uint64_t(count)*32 > bytes.size()) throw std::runtime_error("invalid ELF program table");
    struct Segment { std::uint32_t offset, address, file_size, memory_size; };
    std::vector<Segment> segments;
    bool executable_entry=false;
    for (unsigned i=0;i<count;++i) {
        const auto p=std::size_t(ph)+i*32;
        if (get(p,4)!=1) continue;
        Segment s{get(p+4,4),get(p+8,4),get(p+16,4),get(p+20,4)};
        if (s.file_size > s.memory_size || std::uint64_t(s.offset)+s.file_size > bytes.size()
            || std::uint64_t(s.address)+s.memory_size > state.ram.size()) throw std::runtime_error("invalid ELF load segment");
        for (const auto& t:segments)
            if (s.memory_size && t.memory_size && s.address < t.address+t.memory_size && t.address < s.address+s.memory_size)
                throw std::runtime_error("overlapping ELF segments");
        if ((get(p+24,4)&1) && entry%4==0 && entry>=s.address && std::uint64_t(entry)+4<=std::uint64_t(s.address)+s.file_size)
            executable_entry=true;
        segments.push_back(s);
    }
    if (!executable_entry) throw std::runtime_error("ELF entry is not executable");
    // Transactional validation: do not change state until all headers are checked.
    for (const auto& s:segments) {
        std::copy_n(bytes.begin()+s.offset,s.file_size,state.ram.begin()+s.address);
        std::fill(state.ram.begin()+s.address+s.file_size,state.ram.begin()+s.address+s.memory_size,0);
    }
    state.pc=entry;
    return entry;
}

std::uint32_t load_elf_file(State& state, const std::filesystem::path& path, const std::string& hash) {
    std::ifstream file(path,std::ios::binary|std::ios::ate);
    if (!file) throw std::runtime_error("cannot open local ELF");
    const auto size=file.tellg();
    if (size < 0 || size > 64*1024*1024) throw std::runtime_error("invalid/oversized ELF file");
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(bytes.data()),static_cast<std::streamsize>(bytes.size())))
        throw std::runtime_error("ELF read failed");
    return load_elf(state,bytes,hash);
}
}
