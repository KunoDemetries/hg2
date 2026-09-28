#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>

namespace hg {
// HG-DIAG-004: the submit ring changes at insertion/reset (cursor changes)
// and when an outstanding entry returns. Arm metadata does not affect the
// timing/SNDDRV consumers. Preserve their original scan and observation slice,
// but avoid repeatedly searching already completed, unchanged entries.
template<std::size_t Count> class DiagnosticReturnRefresh {
    static_assert(Count>0,"a diagnostic ring must have entries");
    std::size_t cursor_=std::numeric_limits<std::size_t>::max();
    std::array<std::size_t,Count> pending_{};
    std::size_t pending_count_=0;
public:
    template<class Trace>
    bool needs_scan(const std::array<Trace,Count>& traces,std::size_t cursor) {
        bool changed=cursor!=cursor_;
        for(std::size_t n=0;!changed && n<pending_count_;++n)
            changed=!traces[pending_[n]].valid || traces[pending_[n]].return_seen;
        if(!changed)return false;
        cursor_=cursor;pending_count_=0;
        const auto count=std::min(cursor,Count);
        for(std::size_t n=0;n<count;++n) {
            const auto index=(cursor+Count-count+n)%Count;
            if(traces[index].valid && !traces[index].return_seen)
                pending_[pending_count_++]=index;
        }
        return true;
    }
};
}
