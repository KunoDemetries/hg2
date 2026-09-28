#pragma once

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <thread>
#include <chrono>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>
#endif

namespace hg {

// Host presentation only. This consumes already-modeled 48 kHz stereo PCM and
// never feeds timing or state back into the guest.
class HostAudioOutput {
public:
    static constexpr unsigned sample_rate=48000;
    static constexpr unsigned channels=2;
    // HG-DIAG-014 host presentation buffering. Keep enough queued PCM to absorb
    // normal Windows scheduling / preview stalls without feeding that host jitter
    // back into the guest's already-verified 48 kHz stream timing.
    static constexpr unsigned frames_per_buffer=1024;
    static constexpr unsigned buffer_count=8;
    static constexpr unsigned prebuffer_count=4;
    static constexpr int gain_numerator=3;
    static constexpr int gain_denominator=4;

    HostAudioOutput() {
#ifdef _WIN32
        WAVEFORMATEX format{};
        format.wFormatTag=WAVE_FORMAT_PCM;
        format.nChannels=channels;
        format.nSamplesPerSec=sample_rate;
        format.wBitsPerSample=16;
        format.nBlockAlign=channels*sizeof(std::uint16_t);
        format.nAvgBytesPerSec=format.nSamplesPerSec*format.nBlockAlign;
        const auto result=waveOutOpen(&device_,WAVE_MAPPER,&format,0,0,CALLBACK_NULL);
        if(result!=MMSYSERR_NOERROR)throw std::runtime_error(error_text("cannot open Windows audio output",result));
        const auto paused=waveOutPause(device_);
        if(paused!=MMSYSERR_NOERROR) {
            waveOutClose(device_);device_=nullptr;
            throw std::runtime_error(error_text("cannot prebuffer Windows audio output",paused));
        }
        for(unsigned n=0;n<buffer_count;++n) {
            auto& header=headers_[n];
            header.lpData=reinterpret_cast<LPSTR>(buffers_[n].data());
            header.dwBufferLength=DWORD(buffers_[n].size()*sizeof(std::uint16_t));
            const auto prepared=waveOutPrepareHeader(device_,&header,sizeof(header));
            if(prepared!=MMSYSERR_NOERROR) {
                for(unsigned prior=0;prior<n;++prior)waveOutUnprepareHeader(device_,&headers_[prior],sizeof(headers_[prior]));
                waveOutClose(device_);device_=nullptr;
                throw std::runtime_error(error_text("cannot prepare Windows audio buffer",prepared));
            }
        }
#else
        throw std::runtime_error("host speaker output is currently implemented only on Windows");
#endif
    }
    HostAudioOutput(const HostAudioOutput&)=delete;
    HostAudioOutput& operator=(const HostAudioOutput&)=delete;
    ~HostAudioOutput() noexcept {
#ifdef _WIN32
        if(!device_)return;
        try {flush();drain();} catch(...) {waveOutReset(device_);}
        for(auto& header:headers_)waveOutUnprepareHeader(device_,&header,sizeof(header));
        waveOutClose(device_);
#endif
    }

    void append(std::uint16_t left,std::uint16_t right) {
#ifdef _WIN32
        observe_queue();
        wait_until_free(write_buffer_);
        auto& buffer=buffers_[write_buffer_];
        const auto output_left=scale_sample(left),output_right=scale_sample(right);
        observe_peak(left,input_peak_);observe_peak(right,input_peak_);
        observe_peak(output_left,output_peak_);observe_peak(output_right,output_peak_);
        buffer[write_frames_*2]=output_left;
        buffer[write_frames_*2+1]=output_right;
        ++write_frames_;++frames_;
        if(write_frames_==frames_per_buffer)submit();
#else
        (void)left;(void)right;
#endif
    }

    void flush() {
#ifdef _WIN32
        if(write_frames_) {
            auto& buffer=buffers_[write_buffer_];
            for(unsigned frame=write_frames_;frame<frames_per_buffer;++frame) {
                buffer[frame*2]=0;buffer[frame*2+1]=0;
            }
            submit();
        }
        if(!started_ && submitted_buffers_)start_playback();
#endif
    }

    std::uint64_t frames() const noexcept {return frames_;}
    std::uint64_t underruns() const noexcept {return underruns_;}
    unsigned input_peak() const noexcept {return input_peak_;}
    unsigned output_peak() const noexcept {return output_peak_;}
    unsigned max_queued_buffers() const noexcept {
#ifdef _WIN32
        return max_queued_buffers_;
#else
        return 0;
#endif
    }
    bool started() const noexcept {
#ifdef _WIN32
        return started_;
#else
        return false;
#endif
    }

private:
#ifdef _WIN32
    static std::int32_t signed_sample(std::uint16_t value) {
        return value<0x8000u?std::int32_t(value):std::int32_t(value)-0x10000;
    }
    static std::uint16_t scale_sample(std::uint16_t value) {
        const auto sample=signed_sample(value);
        const auto scaled=sample*gain_numerator/gain_denominator;
        return std::uint16_t(std::int16_t(scaled));
    }
    static void observe_peak(std::uint16_t value,unsigned& peak) {
        const auto sample=signed_sample(value);
        const auto magnitude=unsigned(sample<0?-sample:sample);
        if(magnitude>peak)peak=magnitude;
    }
    static std::string error_text(const char* prefix,MMRESULT result) {
        char text[MAXERRORLENGTH]{};
        if(waveOutGetErrorTextA(result,text,MAXERRORLENGTH)==MMSYSERR_NOERROR)
            return std::string(prefix)+": "+text;
        return std::string(prefix)+" (MMRESULT "+std::to_string(result)+")";
    }
    void wait_until_free(unsigned index) {
        auto& header=headers_[index];
        while(header.dwFlags&WHDR_INQUEUE) {
            observe_queue();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    unsigned queued_buffers() const {
        unsigned queued=0;
        for(const auto& header:headers_)queued+=(header.dwFlags&WHDR_INQUEUE)!=0;
        return queued;
    }
    void observe_queue() {
        const auto queued=queued_buffers();
        if(queued>max_queued_buffers_)max_queued_buffers_=queued;
        if(started_ && !queued) {
            if(!starved_)++underruns_;
            starved_=true;
        } else if(queued)starved_=false;
    }
    void start_playback() {
        const auto result=waveOutRestart(device_);
        if(result!=MMSYSERR_NOERROR)throw std::runtime_error(error_text("cannot start Windows audio output",result));
        started_=true;starved_=false;observe_queue();
    }
    void submit() {
        auto& header=headers_[write_buffer_];
        const auto result=waveOutWrite(device_,&header,sizeof(header));
        if(result!=MMSYSERR_NOERROR)throw std::runtime_error(error_text("cannot submit Windows audio buffer",result));
        ++submitted_buffers_;
        write_buffer_=(write_buffer_+1)%buffer_count;write_frames_=0;
        observe_queue();
        if(!started_ && submitted_buffers_>=prebuffer_count)start_playback();
    }
    void drain() {
        for(unsigned n=0;n<buffer_count;++n)wait_until_free(n);
    }
    HWAVEOUT device_=nullptr;
    std::array<std::array<std::uint16_t,frames_per_buffer*channels>,buffer_count> buffers_{};
    std::array<WAVEHDR,buffer_count> headers_{};
    unsigned write_buffer_=0,write_frames_=0;
    unsigned submitted_buffers_=0,max_queued_buffers_=0;
    unsigned input_peak_=0,output_peak_=0;
    bool started_=false,starved_=false;
#endif
    std::uint64_t frames_=0;
    std::uint64_t underruns_=0;
#ifndef _WIN32
    unsigned input_peak_=0,output_peak_=0;
#endif
};

}
