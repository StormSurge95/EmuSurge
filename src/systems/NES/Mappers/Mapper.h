#pragma once

#include <cstdint>
#include <vector>

class Cartridge;

class Mapper {
    public:
        Mapper() {};
        Mapper(uint8_t prgBanks, std::vector<uint8_t>& prgMemory, uint8_t chrBanks, std::vector<uint8_t>& chrMemory) {
            this->prgBanks = prgBanks;
            this->prgMemory = &prgMemory;
            this->chrBanks = chrBanks;
            this->chrMemory = &chrMemory;
        }

        virtual ~Mapper() = default;

        virtual uint8_t cpuRead(uint16_t addr, bool readonly = false) = 0;
        virtual void cpuWrite(uint16_t addr, uint8_t data) = 0;

        virtual uint8_t ppuRead(uint16_t addr, bool readonly = false) = 0;
        virtual void ppuWrite(uint16_t addr, uint8_t data) = 0;

        virtual void clock() {};

        static const unsigned int PRG_ROM_PAGE_SIZE = 16384U;
        static const unsigned int CHR_ROM_PAGE_SIZE = 8192U;

    protected:
        uint8_t prgBanks = 0;
        uint8_t chrBanks = 0;
        bool hasPrgRam = false;
        bool hasChrRam = false;
        std::vector<uint8_t>* prgMemory = nullptr;
        std::vector<uint8_t>* chrMemory = nullptr;
};