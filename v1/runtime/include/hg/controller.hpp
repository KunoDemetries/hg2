#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
namespace hg {
// Stateful virtual DualShock2 configuration. Wire constants come from the
// independently documented controller protocol, not emulator code.
struct Controller {
    bool configuration=false,analog=false,analog_locked=false,pressure=false;
    std::array<std::uint8_t,12> pressures{}; // Right, Left, Up, Down, triangle, circle, cross, square, L1, R1, L2, R2
    std::array<std::uint8_t,4> axes{{0x80,0x80,0x80,0x80}}; // RX, RY, LX, LY
    std::array<std::uint8_t,6> rumble_map{{0xff,0xff,0xff,0xff,0xff,0xff}};
    std::uint8_t small_motor=0,large_motor=0;
    std::uint16_t initialized_pressure_sensors=0;
    std::vector<std::uint8_t> exchange(const std::vector<std::uint8_t>& in,std::uint16_t buttons);
};
}
