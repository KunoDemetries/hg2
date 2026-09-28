#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>
#include "hg/runtime.hpp"

namespace hg {
std::string sha256(const std::vector<std::uint8_t>&);
std::uint32_t load_elf(State&, const std::vector<std::uint8_t>&, const std::string& expected_hash);
std::uint32_t load_elf_file(State&, const std::filesystem::path&, const std::string& expected_hash);
const char* compiled_image_sha256();
bool compiled_ee_instruction_trace();
void configure_compiled_image(State&);
void run_burst(State&,std::uint64_t);
}
