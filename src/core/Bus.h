#pragma once

#include <cstdint>
#include <string>

#include "Device.h"

struct Mapping {
    uint16_t start;
    uint16_t end;
    Device* device;
    std::string devName;
};

class Bus {
    public:
        Bus() = default;
        ~Bus() = default;

        virtual uint8_t read(uint16_t addr, bool readonly = false) = 0;
        virtual void write(uint16_t addr, uint8_t data) = 0;
};