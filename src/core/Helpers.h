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

struct  {
    uint8_t toU8(int s8) {
        return s8 & 0xFF;
    }
    int8_t toS8(int u8) {
        return (u8 << 24) >> 24;
    }
    uint16_t toU16(int val) {
        return val & 0xFFFF;
    }
    bool overflows(int u8) {
        return u8 >= 256;
    }
    bool isPositive(int s8) {
        return !((s8 >> 7) & 1);
    }
    bool isNegative(int s8) {
        return !!((s8 >> 7) & 1);
    }
    uint8_t getBit(int num, int pos) {
        return (num >> pos) & 1;
    }
    bool getFlag(int num, int pos) {
        return !!this->getBit(num, pos);
    }
    uint8_t setBit(int u8, int bit, int val) {
        uint8_t mask = 1 << bit;
        return (u8 & ~mask) | ((val & 0b1) << bit);
    }
    uint8_t getBits(int u8, int startPos, int size) {
        return (u8 >> startPos) & (0xFF >> (8 - size));
    }
    uint8_t setBits(int u8, int startPos, int size, int val) {
        uint8_t mask = ((1 << size) - 1) << startPos;
        return (u8 & ~mask) | ((val << startPos) & mask);
    }
    uint8_t highByteOf(uint16_t u16) {
        return u16 >> 8;
    }
    uint8_t lowByteOf(uint16_t u16) {
        return u16 & 0xFF;
    }
    uint16_t buildU16(int highByte, int lowByte) {
        return ((highByte & 0xFF) << 8) | (lowByte & 0xFF);
    }
    uint8_t highNybbleOf(uint8_t u8) {
        return u8 >> 4;
    }
    uint8_t lowNybbleOf(uint8_t u8) {
        return u8 & 0b1111;
    }
    uint8_t buildU8(int highNybble, int lowNybble) {
        return ((highNybble & 0b1111) << 4) | (lowNybble & 0b1111);
    }
    uint8_t bitfield(bool bit0, bool bit1, bool bit2, bool bit3, bool bit4, bool bit5, bool bit6, bool bit7) {
        return (
            ((+bit0 & 1) << 0) |
            ((+bit1 & 1) << 1) |
            ((+bit2 & 1) << 2) |
            ((+bit3 & 1) << 3) |
            ((+bit4 & 1) << 4) |
            ((+bit5 & 1) << 5) |
            ((+bit6 & 1) << 6) |
            ((+bit7 & 1) << 7)
        );
    }
    uint8_t buildU2(bool highBit, bool lowBit) {
        return ((+highBit) << 1) | (+lowBit);
    }
} byte;