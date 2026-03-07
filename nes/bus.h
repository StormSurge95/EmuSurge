#pragma once

#include <cstdint>
#include <memory>

#include "cartridge.h"

class CPU;

class Bus {
    public:
        Bus();

        uint8_t cpuRead(uint16_t addr, bool readOnly = false);
        void cpuWrite(uint16_t addr, uint8_t data);

        void insertCartridge(const std::shared_ptr<Cartridge>& cartridge);

        void reset();

        uint8_t cpuRam[2048];

        CPU* cpu = nullptr;

    private:
        std::shared_ptr<Cartridge> cart;
};