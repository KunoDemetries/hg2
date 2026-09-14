#include "hg/iop_image.hpp"
#include "hg/image.hpp"
#include <algorithm>
#include <fstream>
#include <set>

namespace hg {
void load_iop_image(IopState& s,const std::vector<std::uint8_t>& data,const std::string& hash,
                    const std::vector<IopSegment>& segments,const std::vector<IopPatch>& patches) {
    if(data.size()>16*1024*1024 || sha256(data)!=hash)
        throw std::runtime_error("IOP module does not match statically compiled identity");
    std::vector<std::pair<std::uint64_t,std::uint64_t>> ranges;
    for(const auto& part:segments) {
        const auto end=std::uint64_t(part.address)+part.memory_size;
        if(part.file_size>part.memory_size || end>s.ram.size() || part.address%16
           || std::uint64_t(part.offset)+part.file_size>data.size())
            throw std::runtime_error("invalid compiled IOP segment bounds");
        for(const auto& range:ranges)if(part.address<range.second && range.first<end)
            throw std::runtime_error("overlapping compiled IOP segments");
        ranges.push_back({part.address,end});
    }
    std::set<std::uint32_t> patched;
    for(const auto& patch:patches) {
        if(patch.address%4 || !patched.insert(patch.address).second)
            throw std::runtime_error("unaligned or duplicate compiled IOP patch");
        bool matched=false;
        for(const auto& part:segments) {
            if(patch.address<part.address || std::uint64_t(patch.address)+4>std::uint64_t(part.address)+part.file_size)continue;
            const auto offset=part.offset+patch.address-part.address;
            std::uint32_t original=0;
            for(unsigned n=0;n<4;++n)original|=std::uint32_t(data[offset+n])<<(n*8);
            if(original!=patch.before)throw std::runtime_error("compiled IOP relocation source mismatch");
            matched=true;break;
        }
        if(!matched)throw std::runtime_error("compiled IOP relocation outside file-backed segment");
    }
    // All input and patch checks finish before mutating guest memory.
    for(const auto& part:segments) {
        std::copy_n(data.begin()+part.offset,part.file_size,s.ram.begin()+part.address);
        std::fill(s.ram.begin()+part.address+part.file_size,s.ram.begin()+part.address+part.memory_size,0);
    }
    for(const auto& patch:patches)s.store(patch.address,4,patch.after);
}
static std::vector<std::uint8_t> read_file(const std::filesystem::path& path,std::size_t limit) {
    std::ifstream f(path,std::ios::binary|std::ios::ate);
    if(!f)throw std::runtime_error("cannot open local IOP module");
    const auto size=f.tellg();
    if(size<0 || std::uint64_t(size)>limit)throw std::runtime_error("IOP module file size outside limit");
    std::vector<std::uint8_t> data(static_cast<std::size_t>(size));f.seekg(0);
    if(!f.read(reinterpret_cast<char*>(data.data()),data.size()))throw std::runtime_error("short IOP module read");
    return data;
}
void load_iop_file(IopState& s,const std::filesystem::path& path,const std::string& hash,
                   const std::vector<IopSegment>& segments,const std::vector<IopPatch>& patches) {
    load_iop_image(s,read_file(path,16*1024*1024),hash,segments,patches);
}
void load_iop_region(IopState& s,const std::vector<std::uint8_t>& container,const std::string& container_hash,
                     std::uint32_t offset,std::uint32_t size,const std::string& module_hash,
                     const std::vector<IopSegment>& segments,const std::vector<IopPatch>& patches) {
    if(container.size()>64*1024*1024 || size>16*1024*1024 || std::uint64_t(offset)+size>container.size())
        throw std::runtime_error("compiled IOP container region outside bounds");
    if(sha256(container)!=container_hash)throw std::runtime_error("IOP container identity mismatch");
    const std::vector<std::uint8_t> module(container.begin()+offset,container.begin()+offset+size);
    load_iop_image(s,module,module_hash,segments,patches);
}
void load_iop_region_file(IopState& s,const std::filesystem::path& path,const std::string& container_hash,
                          std::uint32_t offset,std::uint32_t size,const std::string& module_hash,
                          const std::vector<IopSegment>& segments,const std::vector<IopPatch>& patches) {
    load_iop_region(s,read_file(path,64*1024*1024),container_hash,offset,size,module_hash,segments,patches);
}
void load_iop_buffered_region_file(IopState& s,const std::filesystem::path& path,const std::string& container_hash,
                          std::uint32_t offset,std::uint32_t size,const std::string& module_hash,
                          const std::vector<IopSegment>& segments,const std::vector<IopPatch>& patches,
                          std::uint32_t base,std::uint32_t prefix) {
    const auto container=read_file(path,64*1024*1024);
    auto staged=std::make_unique<IopState>(s);
    load_iop_region(*staged,container,container_hash,offset,size,module_hash,segments,patches);
    std::uint64_t end=base;
    for(const auto& part:segments) {
        if(part.address<base)throw std::runtime_error("buffered module segment precedes its base");
        end=std::max(end,std::uint64_t(part.address)+part.memory_size);
    }
    const std::vector<std::uint8_t> image(container.begin()+offset,container.begin()+offset+size);
    staged->configure_buffered_iop_module(module_hash,base,std::uint32_t(end-base),prefix,image);
    s=std::move(*staged);
}
}
