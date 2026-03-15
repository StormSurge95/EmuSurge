#pragma once

#include "Mapper.h"
#include <vector>

class M003 : public Mapper {
    public:
        M003(uint8_t prgBanks, std::vector<uint8_t>& prgMemory, uint8_t chrBanks, std::vector<uint8_t>& chrMemory) :
            Mapper(prgBanks, prgMemory, chrBanks, chrMemory) {};
        ~M003() = default;

        uint8_t cpuRead(uint16_t addr, bool readonly = false) override {
            if (addr >= 0x8000 && addr <= 0xFFFF) {
                return (*prgMemory)[addr & (prgBanks == 1 ? 0x3FFF : 0x7FFF)];
            }
            return 0x00;
        }

        void cpuWrite(uint16_t addr, uint8_t data) override {
            if (addr >= 0x8000 && addr <= 0xFFFF) {
                bankSelect = data % chrBanks;
            }
        }

        uint8_t ppuRead(uint16_t addr, bool readonly = false) override {
            if (addr < 0x2000 && chrBanks > 0)
                return (*chrMemory)[(size_t)bankSelect * 0x2000 + (size_t)addr];

            return 0x00;
        }

        void ppuWrite(uint16_t addr, uint8_t data) override {

        }

    private:
        uint8_t bankSelect = 0;
};