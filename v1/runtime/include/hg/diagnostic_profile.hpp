#pragma once
#include <array>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <map>
#include <vector>

namespace hg {
// Host-only provenance of contiguous, successfully consumed disc sectors.
class DiagnosticDiscProfile {
    bool enabled_;
    std::uint32_t first_=0,last_=0;
    std::uint64_t count_=0,first_us_=0,last_us_=0;
    void report() const {
        if(count_)std::cerr<<"Disc read range first_lba="<<first_<<" last_lba="<<last_
            <<" sectors="<<count_<<" first_us="<<first_us_<<" last_us="<<last_us_<<'\n';
    }
public:
    explicit DiagnosticDiscProfile(bool enabled):enabled_(enabled){}
    void consumed(std::uint32_t lba,std::uint64_t us) {
        if(!enabled_)return;
        if(count_ && std::uint64_t(last_)+1!=lba){report();count_=0;}
        if(!count_){first_=lba;first_us_=us;}
        last_=lba;last_us_=us;++count_;
    }
    ~DiagnosticDiscProfile(){report();}
};
// Host-only sampled measurements. No guest clock or scheduling changes.
class DiagnosticProfile {
    using Clock=std::chrono::steady_clock;
    bool enabled_;
    Clock::time_point start_=Clock::now();
    std::array<std::uint64_t,11> ns_{};
    std::map<std::uint32_t,std::uint64_t> ee_pcs_;
    std::map<std::uint32_t,std::uint64_t> ee_cost_;
    std::uint64_t samples_=0,slices_=0;
public:
    explicit DiagnosticProfile(bool enabled):enabled_(enabled){}
    static bool selected(std::uint64_t slice) {
        // Mix the index to avoid phase-locking with periodic guest loops.
        auto x=slice+0x9e3779b97f4a7c15ull;
        x=(x^(x>>30))*0xbf58476d1ce4e5b9ull;
        x=(x^(x>>27))*0x94d049bb133111ebull;
        return ((x^(x>>31))&255)==0;
    }
    class Slice {
        DiagnosticProfile& owner_;
        bool sampled_;
        unsigned phase_=0;
        std::uint32_t entry_pc_=0;
        Clock::time_point previous_{};
    public:
        Slice(DiagnosticProfile& owner,std::uint64_t slice,std::uint32_t pc)
            :owner_(owner),sampled_(owner.enabled_ && selected(slice)),entry_pc_(pc) {
            if(owner.enabled_)owner.slices_=slice+1;
            if(sampled_) {
                ++owner.samples_;++owner.ee_pcs_[pc];previous_=Clock::now();
            }
        }
        void mark(unsigned next) {
            if(sampled_) {
                const auto now=Clock::now();
                const auto elapsed=std::chrono::duration_cast<std::chrono::nanoseconds>(now-previous_).count();
                owner_.ns_[phase_]+=elapsed;
                if(phase_==2)owner_.ee_cost_[entry_pc_]+=elapsed;
                previous_=now;phase_=next;
            }
        }
        ~Slice(){mark(0);}
    };
    void report() const {
        if(!enabled_)return;
        const auto seconds=std::chrono::duration<double>(Clock::now()-start_).count();
        constexpr const char* names[]{"host_io","clocks","ee_run","ee_ipu_input",
                                     "ee_ipu_output","ee_vif_gif","ee_raster","iop_run_io",
                                     "diagnostic_history","sif","interrupts_bootstrap"};
        std::cerr<<"Host profile slices="<<slices_<<" seconds="<<seconds
                 <<" slices_per_second="<<(seconds?slices_/seconds:0)<<" samples="<<samples_<<'\n';
        for(unsigned n=0;n<ns_.size();++n)std::cerr<<"Profile phase "<<names[n]<<" sampled_ns="<<ns_[n]<<'\n';
        std::vector<std::pair<std::uint64_t,std::uint32_t>> pcs;
        for(const auto& [pc,count]:ee_pcs_)pcs.emplace_back(count,pc);
        std::sort(pcs.rbegin(),pcs.rend());
        for(unsigned n=0;n<std::min<std::size_t>(16,pcs.size());++n)
            std::cerr<<"Profile EE boundary pc=0x"<<std::hex<<pcs[n].second<<std::dec<<" samples="<<pcs[n].first<<'\n';
        pcs.clear();
        for(const auto& [pc,ns]:ee_cost_)pcs.emplace_back(ns,pc);
        std::sort(pcs.rbegin(),pcs.rend());
        for(unsigned n=0;n<std::min<std::size_t>(16,pcs.size());++n)
            std::cerr<<"Profile EE quantum-entry pc=0x"<<std::hex<<pcs[n].second<<std::dec<<" sampled_ns="<<pcs[n].first<<'\n';
    }
    ~DiagnosticProfile(){report();}
};
}
