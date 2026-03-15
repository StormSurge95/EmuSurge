#pragma once

#include "Mapper.h"

class M000 : public Mapper {
    public:
        M000(uint8_t prgBanks, std::vector<uint8_t>& prgMemory, uint8_t chrBanks, std::vector<uint8_t>& chrMemory) {
            this->prgBanks = prgBanks;
            this->prgMemory = &prgMemory;
            this->chrBanks = chrBanks;
            this->chrMemory = &chrMemory;
        }

        uint8_t cpuRead(uint16_t addr, bool readonly = false) override {
            if (addr >= 4020 && addr <= 0x7FFF) {
                // Unused
                return 0x00;
            } else if (addr >= 0x8000) {
                addr = addr & (this->prgBanks > 1 ? 0x7FFF : 0x3FFF);
                return (*this->prgMemory)[addr];
            }
        }

        void cpuWrite(uint16_t addr, uint8_t data) override {
            // Unused
            return;
        }

        uint8_t ppuRead(uint16_t addr, bool readonly = false) override {
            if (addr <= 0x1FFF)
                return (*this->chrMemory)[addr];
            return 0x00;
        }

        void ppuWrite(uint16_t addr, uint8_t data) override {
            if (this->chrBanks > 0) return; // only CHR-RAM is writable

            if (addr <= 0x1FFF)
                (*this->chrMemory)[addr] = data;
        }
};