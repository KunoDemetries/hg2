#pragma once
#include <cstdint>

namespace hg {
// Native launch preferences. Defaults: English, 4:3, RGB, SPDIF on, UTC.
// ConfigParam layout comes from the public ABI, not a captured memory image.
struct ConsoleSettings {
    unsigned spdif_disabled=0, screen_type=0, component_output=0;
    unsigned non_japanese=1, ps1_driver=0, version=1, language=1, timezone=0;
    std::uint32_t pack() const {
        return (spdif_disabled&1) | ((screen_type&3)<<1) | ((component_output&1)<<3)
            | ((non_japanese&1)<<4) | ((ps1_driver&255)<<5) | ((version&7)<<13)
            | ((language&31)<<16) | ((timezone&2047)<<21);
    }
    void unpack(std::uint32_t value) {
        spdif_disabled=value&1; screen_type=value>>1&3; component_output=value>>3&1;
        non_japanese=value>>4&1; ps1_driver=value>>5&255; version=value>>13&7;
        language=value>>16&31; timezone=value>>21;
    }
};
}
