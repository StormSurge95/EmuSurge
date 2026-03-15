#pragma once

#include "Mapper.h"

class M001 : public Mapper {
    public:
        M001(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks) {}

        uint8_t cpuRead(uint16_t addr, bool readonly = false) override {
            uint8_t data = 0x00;
            if (addr >= 0x8000) {
                if (prgMode == 0 || prgMode == 1) {
                    uint16_t bank = (prgBank & 0x0E);
                    data = (bank * 0x4000) + (addr & 0x7FFF);
                } else if (prgMode == 2) {
                    if (addr < 0xC000)
                        data = addr & 0x3FFF;
                    else
                        data = (prgBank * 0x4000) + (addr & 0x3FFF);
                } else if (prgMode == 3) {
                    if (addr < 0xC000)
                        data = (prgBank * 0x4000) + (addr & 0x3FFF);
                    else
                        data = ((prgBanks - 1) * 0x4000) + (addr & 0x3FFF);
                }
            }

            return data;
        }

        void cpuWrite(uint16_t addr, uint8_t data) override {
            if (writeLatch)
                return;

            writeLatch = true;

            if (data & 0x80) {
                shiftRegister = 0x10;
                control |= 0x0C;
                return;
            }

            bool complete = shiftRegister & 1;

            shiftRegister >>= 1;
            shiftRegister |= (data & 1) << 4;

            if (complete) {
                uint8_t target = (addr >> 13) & 3;

                switch (target) {
                    case 0:
                        control = shiftRegister & 0x1F;
                        chrMode = (control >> 4) & 1;
                        prgMode = (control >> 2) & 3;
                        break;
                    case 1:
                        chrBank0 = shiftRegister & 0x1F;
                        break;
                    case 2:
                        chrBank1 = shiftRegister & 0x1F;
                        break;
                    case 3:
                        prgBank = shiftRegister & 0x0F;
                        break;
                }

                shiftRegister = 0x10;
            }
        }

        uint8_t ppuRead(uint16_t addr, bool readonly = false) override {
            uint8_t data = 0x00;

            if (addr < 0x2000) {
                if (chrMode == 0)
                    data = ((chrBanks & 0x1E) * 0x1000) + addr;
                else if (addr < 0x1000)
                    data = (chrBank0 * 0x1000) + (addr & 0x0FFF);
                else
                    data = (chrBank1 * 0x1000) + (addr & 0x0FFF);
            }

            return data;
        }

        void ppuWrite(uint16_t addr, uint8_t data) override {
            if (addr < 0x2000) {
                if (chrMode == 0)
                    data = ((chrBank0 & 0x1E) * 0x1000) + addr;
                else if (addr < 0x1000)
                    data = (chrBank0 * 0x1000) + (addr & 0x0FFF);
                else
                    data = (chrBank1 * 0x1000) + (addr & 0x0FFF);
            }
        }

        void clock() override {
            writeLatch = false;
        }

    private:
        uint8_t shiftRegister = 0x10;
        uint8_t control = 0x1C;

        uint8_t chrBank0 = 0;
        uint8_t chrBank1 = 0;
        uint8_t prgBank = 0;

        uint8_t prgMode = 3;
        uint8_t chrMode = 0;

        bool writeLatch = false;
        int a = 0x7FFF;
};