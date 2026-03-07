#pragma once

#include <iomanip>
#include <sstream>
#include <string>

inline std::string hex(uint32_t value, uint8_t width = 2) {
    std::stringstream ss;
    ss << std::uppercase << std::setfill('0')
        << std::setw(width) << std::hex << value;
    return ss.str();
}

inline std::string padRight(const std::string& s, size_t width) {
    if (s.length() >= width) return s;
    return s + std::string(width - s.length(), ' ');
}