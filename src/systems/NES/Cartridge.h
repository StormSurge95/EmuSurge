#pragma once

#include "../../core/Device.h"
#include "Mappers/Mapper.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

struct INESHeader {
    char name[4];       // "NES<EOF>"
    uint8_t prgChunks;  // num 16KB PRG banks
    uint8_t chrChunks;  // num 8KB CHR banks
    uint8_t flags6;     // MMMMntrm
    uint8_t flags7;     // MMMM----
    uint8_t prgRamSize;
    uint8_t tvSystem1;
    uint8_t tvSystem2;
    char unused[5];
};

class Cartridge : public Device {
    public:
        enum MIRROR {
            HORIZONTAL,
            VERTICAL,
            FOUR_SCREEN
        };

        std::shared_ptr<Mapper> mapper = nullptr;

        Cartridge(const std::string& filename);

        void clock() { mapper->clock(); };

        bool isValid() const { return valid; }

        uint8_t read(uint16_t addr, bool readonly = false) override;
        void write(uint16_t addr, uint8_t data) override;

        uint8_t ppuRead(uint16_t addr, bool readonly = false);
        void ppuWrite(uint16_t addr, uint8_t data);

        inline MIRROR getMirror() const { return mirror; }

        std::vector<uint8_t> prg() { return prgMemory; }
        std::vector<uint8_t> chr() { return chrMemory; }

    private:
        bool valid = false;

        uint8_t mapperID = 0;
        uint8_t prgBanks = 0;
        uint8_t chrBanks = 0;
        MIRROR mirror = HORIZONTAL;

        std::vector<uint8_t> prgMemory{ 0 };
        std::vector<uint8_t> chrMemory{ 0 };

        void initMapper(uint8_t mapperID);
};