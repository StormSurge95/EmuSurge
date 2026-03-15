#pragma once

#include <cstdint>

class Device {
    public:
        virtual ~Device() = default;

        virtual uint8_t read(uint16_t addr, bool readonly = false) = 0;
        virtual void write(uint16_t addr, uint8_t data) = 0;
};