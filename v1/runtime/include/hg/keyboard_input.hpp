#pragma once
#include "hg/native_host.hpp"
#include <cstddef>

namespace hg {
// Physical keyboard keys, independent of GLFW so the actual mapping is testable.
// The first sixteen entries follow the existing DualShock2 button-bit order.
enum class KeyboardKey : std::size_t {
    backspace, left_shift, right_shift, enter,
    up, right, down, left, digit1, digit3, q, e, v, x, space, c,
    a, d, w, s, j, l, i, k, count
};
inline constexpr std::size_t keyboard_key_count=static_cast<std::size_t>(KeyboardKey::count);
using KeyboardKeys=std::array<bool,keyboard_key_count>;

inline HostInput keyboard_input(const KeyboardKeys& keys, bool focused=true) {
    HostInput input;
    if(!focused)return input; // Focus loss releases buttons, pressure and both sticks.
    constexpr std::array<int,16> pressure_for_bit{{-1,-1,-1,-1,2,0,3,1,10,11,8,9,4,5,6,7}};
    for(unsigned bit=0;bit<16;++bit)if(keys[bit]) {
        input.buttons&=std::uint16_t(~(1u<<bit));
        const int pressure=pressure_for_bit[bit];
        if(pressure>=0)input.pressures[static_cast<unsigned>(pressure)]=255;
    }
    const auto axis=[&](KeyboardKey negative,KeyboardKey positive) -> std::uint8_t {
        const bool n=keys[static_cast<std::size_t>(negative)],p=keys[static_cast<std::size_t>(positive)];
        return n==p?128:n?0:255;
    };
    // Wire order is RX, RY, LX, LY. Keyboard axes are digital endpoints;
    // connected gamepads retain their analog values in the host's merge path.
    input.axes={{axis(KeyboardKey::j,KeyboardKey::l),axis(KeyboardKey::i,KeyboardKey::k),
                 axis(KeyboardKey::a,KeyboardKey::d),axis(KeyboardKey::w,KeyboardKey::s)}};
    return input;
}
} // namespace hg
