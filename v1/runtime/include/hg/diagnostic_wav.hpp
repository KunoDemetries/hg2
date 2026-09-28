#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace hg {

class DiagnosticWavWriter {
public:
    explicit DiagnosticWavWriter(const std::filesystem::path& path) : output_(path,std::ios::binary|std::ios::trunc) {
        if(!output_)throw std::runtime_error("cannot create SPU2 input WAV");
        write_header(0);
    }
    DiagnosticWavWriter(const DiagnosticWavWriter&)=delete;
    DiagnosticWavWriter& operator=(const DiagnosticWavWriter&)=delete;
    ~DiagnosticWavWriter() noexcept {
        try { finalize(); } catch(...) {}
    }

    void append(std::uint16_t left,std::uint16_t right) {
        if(finalized_)throw std::runtime_error("SPU2 input WAV already finalized");
        if(frames_==std::numeric_limits<std::uint32_t>::max()/4u)
            throw std::runtime_error("SPU2 input WAV exceeds RIFF size limit");
        write_u16(left);write_u16(right);++frames_;
        if(!output_)throw std::runtime_error("cannot write SPU2 input WAV");
    }

    std::uint32_t frames() const noexcept {return frames_;}

    void finalize() {
        if(finalized_)return;
        const auto end=output_.tellp();
        if(end<0)throw std::runtime_error("cannot finalize SPU2 input WAV");
        output_.seekp(0);
        write_header(frames_);
        output_.seekp(end);
        output_.flush();
        if(!output_)throw std::runtime_error("cannot finalize SPU2 input WAV");
        finalized_=true;
    }

private:
    void write_u16(std::uint16_t value) {
        const char bytes[2]={char(value&0xffu),char((value>>8)&0xffu)};
        output_.write(bytes,2);
    }
    void write_u32(std::uint32_t value) {
        const char bytes[4]={char(value&0xffu),char((value>>8)&0xffu),char((value>>16)&0xffu),char((value>>24)&0xffu)};
        output_.write(bytes,4);
    }
    void write_header(std::uint32_t frames) {
        const std::uint32_t data_bytes=frames*4u;
        output_.write("RIFF",4);write_u32(36u+data_bytes);output_.write("WAVE",4);
        output_.write("fmt ",4);write_u32(16);write_u16(1);write_u16(2);
        write_u32(48000);write_u32(48000u*4u);write_u16(4);write_u16(16);
        output_.write("data",4);write_u32(data_bytes);
    }

    std::ofstream output_;
    std::uint32_t frames_=0;
    bool finalized_=false;
};

}
