#pragma once
#include "hg/iop.hpp"
#include <filesystem>

namespace hg {
struct IopSegment {std::uint32_t offset,address,file_size,memory_size;};
struct IopPatch {std::uint32_t address,before,after;};
void load_iop_image(IopState&,const std::vector<std::uint8_t>&,const std::string&,
                    const std::vector<IopSegment>&,const std::vector<IopPatch>&);
void load_iop_file(IopState&,const std::filesystem::path&,const std::string&,
                   const std::vector<IopSegment>&,const std::vector<IopPatch>&);
void load_iop_region(IopState&,const std::vector<std::uint8_t>&,const std::string&,
                     std::uint32_t,std::uint32_t,const std::string&,
                     const std::vector<IopSegment>&,const std::vector<IopPatch>&);
void load_iop_region_file(IopState&,const std::filesystem::path&,const std::string&,
                          std::uint32_t,std::uint32_t,const std::string&,
                          const std::vector<IopSegment>&,const std::vector<IopPatch>&);
void load_iop_buffered_region_file(IopState&,const std::filesystem::path&,const std::string&,
                          std::uint32_t,std::uint32_t,const std::string&,
                          const std::vector<IopSegment>&,const std::vector<IopPatch>&,
                          std::uint32_t,std::uint32_t);
void run_iop(IopState&,std::uint64_t);
void run_iop_burst(IopState&,std::uint64_t);
void load_compiled_iop(IopState&,const std::filesystem::path&);
const char* compiled_iop_name();
std::uint32_t compiled_iop_entry();
std::uint32_t compiled_iop_export(unsigned);
std::uint32_t compiled_iop_library_export(const char*,unsigned);
std::uint32_t compiled_iop_module_export(const char*,const char*,unsigned);
std::uint32_t compiled_iop_module_entry(const char*);
std::uint32_t compiled_iop_module_base(const char*);
std::uint32_t compiled_iop_module_memory_size(const char*);
std::uint32_t compiled_iop_module_allocation_prefix(const char*);
std::uint32_t compiled_iop_high_water_mark();
void link_compiled_iop_module(IopState&,std::uint32_t,std::uint32_t);
std::uint32_t compiled_iop_table(const char*,const char*,bool);
}
