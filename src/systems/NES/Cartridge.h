#pragma once

#include "../../core/Device.h"

#include <cstdint>
#include <string>
#include <vector>

struct INESHeader {
    char name[4];       // "NES<EOF>"
    uint8_t prgChunks;  // num 16KB PRG banks
    uint8_t chrChunks;  // num 8KB CHR banks
    uint8_t flags6;
    uint8_t flags7;
    uint8_t prgRamSize;
    uint8_t tvSystem1;
    uint8_t tvSystem2;
    char unused[5];
};

class Cartridge : public Device {
    public:
        enum MIRROR {
            HORIZONTAL,
            VERTICAL
        };

        Cartridge(const std::string& filename);

        bool isValid() const { return valid; }

        uint8_t read(uint16_t addr, bool readonly = false) override;
        void write(uint16_t addr, uint8_t data) override;

        bool ppuRead(uint16_t addr, uint8_t& data);
        bool ppuWrite(uint16_t addr, uint8_t data);

        inline MIRROR getMirror() const { return mirror; }

    private:
        bool valid = false;

        uint8_t mapperID = 0;
        uint8_t prgBanks = 0;
        uint8_t chrBanks = 0;
        MIRROR mirror = HORIZONTAL;

        std::vector<uint8_t> prgMemory;
        std::vector<uint8_t> chrMemory;
};