#pragma once
#include <cstdint>
#include <stdexcept>

namespace hg {
// EE User's Manual, I_STAT/I_MASK: latched requests and write-one toggles.
struct InterruptController {
    std::uint32_t status=0, mask=0;
    static bool contains(std::uint32_t address) {
        return address==0x1000f000 || address==0x1000f010;
    }
    std::uint32_t read(std::uint32_t address) const {
        if(address==0x1000f000) return status;
        if(address==0x1000f010) return mask;
        throw std::runtime_error("invalid INTC register");
    }
    void write(std::uint32_t address,std::uint32_t value) {
        if(address==0x1000f000) status&=~(value&0x7fff);
        else if(address==0x1000f010) mask^=value&0x7fff;
        else throw std::runtime_error("invalid INTC register");
    }
    void raise(unsigned cause) {
        if(cause>=15) throw std::runtime_error("invalid INTC cause");
        status|=1u<<cause;
    }
    bool pending() const {return (status&mask)!=0;}
};
}
