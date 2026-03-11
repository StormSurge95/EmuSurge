#pragma once

#include <bitset>
#include <iomanip>
#include <sstream>
#include <string>

inline std::string hex(uint32_t n, uint8_t d = 2) {
    std::stringstream ss;
    ss << std::uppercase << std::setfill('0') << std::setw(d) << std::hex << n;
    return ss.str();
}

inline std::string bin(uint8_t n) {
    std::stringstream ss;
    ss << std::bitset<8>(n);
    return ss.str();

}