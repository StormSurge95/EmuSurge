#ifndef NES_MAPPER
#define NES_MAPPER

#include <cstdint>

class Mapper {
    public:
        Mapper(uint8_t prgBanks, uint8_t chrBanks)
            : prgBanks(prgBanks), chrBanks(chrBanks) {}

        virtual ~Mapper() = default;

        virtual bool cpuMapRead(uint16_t addr, uint32_t& mapped_addr) = 0;
        virtual bool cpuMapWrite(uint16_t addr, uint32_t& mapped_addr) = 0;

    protected:
        uint8_t prgBanks;
        uint8_t chrBanks;
};

#endif