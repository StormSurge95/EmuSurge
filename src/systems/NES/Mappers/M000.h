#pragma once

#include "Mapper.h"

class M000 : public Mapper {
    public:
        M000(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks) {}

        uint8_t cpuRead(uint16_t addr, bool readonly = false) override {
            uint8_t data = 0x00;

            if (addr >= 0x8000) {
                data = addr & (prgBanks > 1 ? 0x7FFF : 0x3FFF);
            }

            return data;
        }

        void cpuWrite(uint16_t addr, uint8_t data) override {
            if (addr >= 0x8000) {
                data = addr & (prgBanks > 1 ? 0x7FFF : 0x3FFF);
            }
        }

        uint8_t ppuRead(uint16_t addr, bool readonly = false) override {
            uint8_t data = 0x00;

            if (addr < 0x2000) {
                data = addr;
            }

            return data;
        }

        void ppuWrite(uint16_t addr, uint8_t data) override {
            if (addr < 0x2000 && chrBanks == 0) {
                data = addr;
            }
        }
};