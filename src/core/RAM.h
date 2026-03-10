#pragma once

#include "Device.h"

#include <vector>

class RAM : public Device {
    public:
        inline RAM(size_t size) {
            memory.resize(size);
        }

        inline uint8_t read(uint16_t addr, bool readonly = false) override {
            return memory[addr & (memory.size() - 1)];
        }

        inline void write(uint16_t addr, uint8_t data) override {
            memory[addr & (memory.size() - 1)] = data;
        }

    private:
        std::vector<uint8_t> memory;
};