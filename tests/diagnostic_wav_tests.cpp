#include "hg/diagnostic_wav.hpp"
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#define CHECK(x) do{if(!(x))throw std::runtime_error(#x);}while(0)

int main() {
    const auto path=std::filesystem::temp_directory_path()/"hg-diagnostic-wav-test.wav";
    std::error_code ignored;std::filesystem::remove(path,ignored);
    {
        hg::DiagnosticWavWriter wav(path);
        wav.append(0x0001,0xffff);
        wav.append(0x8000,0x7fff);
        CHECK(wav.frames()==2);
        wav.finalize();
        CHECK(wav.frames()==2);
    }
    std::ifstream input(path,std::ios::binary);
    std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(input)),{});
    CHECK(bytes.size()==52);
    const std::array<unsigned char,52> expected={
        'R','I','F','F',44,0,0,0,'W','A','V','E','f','m','t',' ',16,0,0,0,1,0,2,0,
        0x80,0xbb,0,0,0,0xee,2,0,4,0,16,0,'d','a','t','a',8,0,0,0,
        1,0,0xff,0xff,0,0x80,0xff,0x7f
    };
    CHECK(std::equal(bytes.begin(),bytes.end(),expected.begin(),expected.end()));
    std::filesystem::remove(path,ignored);
    std::cout<<"Diagnostic WAV tests passed\n";
}
