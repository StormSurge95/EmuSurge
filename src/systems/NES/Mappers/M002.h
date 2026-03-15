#pragma once

#include "Mapper.h"

class M002 : public Mapper {
    public:
        M002(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks) {}

        uint8_t cpuRead(uint16_t addr, bool readonly = false) override {
            uint8_t data = 0x00;

            if (addr >= 0x8000 && addr <= 0xBFFF) {
                data = (prgBankSelect * 0x4000) + (addr & 0x3FFF);
            }

            if (addr >= 0xC000) {
                data = ((prgBanks - 1) * 0x4000) + (addr & 0x3FFF);
            }

            return data;
        }

        void cpuWrite(uint16_t addr, uint8_t data) override {
            if (addr >= 0x8000)
                prgBankSelect = data & (prgBanks - 1);
        }

        uint8_t ppuRead(uint16_t addr, bool readonly = false) override {
            uint8_t data = 0x00;

            if (addr < 0x2000) {
                data = addr;
            }

            return data;
        }

        void ppuWrite(uint16_t addr, uint8_t data) override {
            if (addr < 0x2000) {
                data = addr;
            }
        }

    private:
        uint8_t prgBankSelect = 0;
};