#include "cartridge.h"
#include "NROM.h"

#include <fstream>
#include <ios>
#include <string>

struct INESHeader {
    char name[4];
    uint8_t prg_rom_chunks;
    uint8_t chr_rom_chunks;
    uint8_t mapper1;
    uint8_t mapper2;
    uint8_t prg_ram_size;
    uint8_t tv_system1;
    uint8_t tv_system2;
    char unused[5];
};

Cartridge::Cartridge(const std::string& filename) {
    std::ifstream ifs(filename, std::ifstream::binary);

    if (!ifs.is_open())
        return;

    INESHeader header;
    ifs.read((char*)&header, sizeof(INESHeader));

    if (header.mapper1 & 0x04)
        ifs.seekg(512, std::ios_base::cur);

    mapperID = ((header.mapper2 >> 4) << 4) | (header.mapper1 >> 4);

    prgBanks = header.prg_rom_chunks;
    chrBanks = header.chr_rom_chunks;

    prgMemory.resize(prgBanks * 16384);
    ifs.read((char*)prgMemory.data(), prgMemory.size());

    chrMemory.resize(chrBanks * 8192);
    ifs.read((char*)chrMemory.data(), chrMemory.size());

    switch (mapperID) {
        case 0:
            mapper = std::make_unique<NROM>(prgBanks, chrBanks);
            break;
    }

    valid = true;
}

bool Cartridge::imageValid() const {
    return valid;
}

bool Cartridge::cpuRead(uint16_t addr, uint8_t& data) {
    uint32_t mapped;

    if (mapper->cpuMapRead(addr, mapped)) {
        data = prgMemory[mapped];
        return true;
    }

    return false;
}

bool Cartridge::cpuWrite(uint16_t addr, uint8_t data) {
    uint32_t mapped;

    if (mapper->cpuMapWrite(addr, mapped)) {
        prgMemory[mapped] = data;
        return true;
    }

    return false;
}