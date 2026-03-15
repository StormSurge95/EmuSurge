#pragma once

#include <cstdint>

class Mapper {
    public:
        Mapper(uint8_t prgBanks, uint8_t chrBanks) : prgBanks(prgBanks), chrBanks(chrBanks) {}

        virtual ~Mapper() = default;

        virtual uint8_t cpuRead(uint16_t addr, bool readonly = false) = 0;
        virtual void cpuWrite(uint16_t addr, uint8_t data) = 0;

        virtual uint8_t ppuRead(uint16_t addr, bool readonly = false) = 0;
        virtual void ppuWrite(uint16_t addr, uint8_t data) = 0;

        virtual void clock() {};

    protected:
        uint8_t prgBanks = 0;
        uint8_t chrBanks = 0;
};