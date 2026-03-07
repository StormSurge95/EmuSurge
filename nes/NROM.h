#ifndef MAPPER_000
#define MAPPER_000

#include "mapper.h"

class NROM : public Mapper {
    public:
        NROM(uint8_t prgBanks, uint8_t chrBanks);

        bool cpuMapRead(uint16_t addr, uint32_t& mapped_addr) override;

        bool cpuMapWrite(uint16_t addr, uint32_t& mapped_addr) override;
};

#endif