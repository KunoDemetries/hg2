#pragma once
#include <cstdint>
#include <cstddef>
#include <deque>
#include <stdexcept>

namespace hg {
// Shared EE/IOP transport state. Initialization flags start clear and may only be
// published by an endpoint after its corresponding transport is actually ready.
struct SifLink {
    struct Quad {std::uint64_t low=0,high=0;};
    std::uint32_t main_address=0,sub_address=0;
    std::uint32_t main_flags=0,sub_flags=0;
    std::uint32_t iop_control=0;
    explicit SifLink(std::size_t capacity=16):capacity_(capacity) {
        if(!capacity)throw std::invalid_argument("SIF FIFO capacity must be positive");
    }
    bool push_main(Quad value) {if(to_main_.size()==capacity_)return false;to_main_.push_back(value);return true;}
    const Quad* peek_main() const {return to_main_.empty()?nullptr:&to_main_.front();}
    void consume_main() {if(to_main_.empty())throw std::runtime_error("empty SIF FIFO");to_main_.pop_front();}
    std::size_t pending_main() const {return to_main_.size();}
    bool push_sub(Quad value) {if(to_sub_.size()==capacity_)return false;to_sub_.push_back(value);return true;}
    const Quad* peek_sub() const {return to_sub_.empty()?nullptr:&to_sub_.front();}
    void consume_sub() {if(to_sub_.empty())throw std::runtime_error("empty SIF FIFO");to_sub_.pop_front();}
    std::size_t pending_sub() const {return to_sub_.size();}
private:
    std::size_t capacity_;
    std::deque<Quad> to_main_,to_sub_;
};
}
