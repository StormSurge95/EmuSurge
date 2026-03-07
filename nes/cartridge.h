#ifndef NES_CARTRIDGE
#define NES_CARTRIDGE

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "mapper.h"

class Cartridge {
    public:
        Cartridge(const std::string& filename);

        bool imageValid() const;

        bool cpuRead(uint16_t addr, uint8_t& data);
        bool cpuWrite(uint16_t addr, uint8_t data);

    private:
        bool valid = false;

        std::vector<uint8_t> prgMemory;
        std::vector<uint8_t> chrMemory;

        uint8_t mapperID = 0;
        uint8_t prgBanks = 0;
        uint8_t chrBanks = 0;

        std::unique_ptr<Mapper> mapper;
};

#endif