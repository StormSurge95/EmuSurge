#pragma once

#include "Mapper.h"

class M001 : public Mapper {
    public:
        M001(uint8_t prgBanks, std::vector<uint8_t>& prgMemory, uint8_t chrBanks, std::vector<uint8_t>& chrMemory) {
            this->prgBanks = prgBanks;
            this->prgMemory = &prgMemory;
            this->chrBanks = chrBanks;
            this->chrMemory = &chrMemory;
        }

        uint8_t cpuRead(uint16_t addr, bool readonly = false) override {
            if (addr >= 0x8000) {
                if (prgMode == 0 || prgMode == 1) {
                    uint16_t bank = (prgBank & 0x0E);
                    addr = (bank * 0x4000) + (addr & 0x7FFF);
                    return (*this->prgMemory)[addr];
                } else if (prgMode == 2) {
                    if (addr < 0xC000) addr = addr & 0x3FFF;
                    else addr = (prgBank * 0x4000) + (addr & 0x3FFF);

                    return (*this->prgMemory)[addr];
                } else if (prgMode == 3) {
                    if (addr < 0xC000) addr = (prgBank * 0x4000) + (addr & 0x3FFF);
                    else addr = ((prgBanks - 1) * 0x4000) + (addr & 0x3FFF);

                    return (*this->prgMemory)[addr];
                }
            }

            return 0x00;
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
            if (addr < 0x2000) {
                if (chrMode == 0) addr = ((chrBanks & 0x1E) * 0x1000) + addr;
                else if (addr < 0x1000) addr = (chrBank0 * 0x1000) + (addr & 0x0FFF);
                else addr = (chrBank1 * 0x1000) + (addr & 0x0FFF);

                return (*this->chrMemory)[addr];
            }

            return 0x00;
        }

        void ppuWrite(uint16_t addr, uint8_t data) override {
            if (addr < 0x2000) {
                if (chrMode == 0) addr = ((chrBank0 & 0x1E) * 0x1000) + addr;
                else if (addr < 0x1000) addr = (chrBank0 * 0x1000) + (addr & 0x0FFF);
                else addr = (chrBank1 * 0x1000) + (addr & 0x0FFF);

                (*this->chrMemory)[addr] = data;
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