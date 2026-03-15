#pragma once

#include "Mapper.h"

class M002 : public Mapper {
    public:
        M002(uint8_t prgBanks, std::vector<uint8_t>& prgMemory, uint8_t chrBanks, std::vector<uint8_t>& chrMemory) {
            this->prgBanks = prgBanks;
            this->prgMemory = &prgMemory;
            this->chrBanks = chrBanks;
            this->chrMemory = &chrMemory;
        }

        uint8_t cpuRead(uint16_t addr, bool readonly = false) override {
            if (addr >= 0x8000 && addr <= 0xBFFF) addr = (prgBankSelect * 0x4000) + (addr & 0x3FFF);
            else if (addr >= 0xC000) addr = ((prgBanks - 1) * 0x4000) + (addr & 0x3FFF);

            return (*this->prgMemory)[addr];
        }

        void cpuWrite(uint16_t addr, uint8_t data) override {
            if (addr >= 0x8000) prgBankSelect = data & (prgBanks - 1);
        }

        uint8_t ppuRead(uint16_t addr, bool readonly = false) override {
            if (addr < 0x2000) {
                return (*this->chrMemory)[addr];
            }

            return 0x00;
        }

        void ppuWrite(uint16_t addr, uint8_t data) override {
            if (addr < 0x2000) {
                (*this->chrMemory)[addr] = data;
            }
        }

    private:
        uint8_t prgBankSelect = 0;
};