#include "Cartridge.h"
#include "Mappers/M000.h"
#include "Mappers/M001.h"
#include "Mappers/M002.h"

#include <fstream>
#include <sstream>

Cartridge::Cartridge(const std::string& filename) {
    // open ROM file
    std::ifstream file(filename, std::ifstream::binary);

    // very file was found and opened
    if (!file) return;

    // read header metadata
    INESHeader header{};
    file.read(reinterpret_cast<char*>(&header), sizeof(INESHeader));

    // verify that ROM is a NES ROM
    if (header.name[0] != 'N' ||
        header.name[1] != 'E' ||
        header.name[2] != 'S' ||
        header.name[3] != 0x1A) return;

    // get number of PRG/CHR banks
    prgBanks = header.prgChunks;
    chrBanks = header.chrChunks;

    // get mapper id
    mapperID = ((header.flags7 >> 4) << 4) | (header.flags6 >> 4);
    initMapper(mapperID);

    mirror = ((header.flags6 & 0x08) ? FOUR_SCREEN : (header.flags6 & 0x01) ? VERTICAL : HORIZONTAL);
    std::stringstream ss;
    ss << "address before trainer search: " << file.tellg() << std::endl;
    if (header.flags6 & 0x04) file.seekg(512, std::ios::cur);
    ss << "address after trainer search:  " << file.tellg() << std::endl;
    printf(ss.str().c_str());

    // read prgMemory
    prgMemory.resize((size_t)prgBanks * 16384);
    file.read(reinterpret_cast<char*>(prgMemory.data()), prgMemory.size());

    if (chrBanks == 0) {
        chrMemory.resize(8192);
        std::fill(chrMemory.begin(), chrMemory.end(), 0);
    } else {
        chrMemory.resize((size_t)chrBanks * 8192);
        file.read(reinterpret_cast<char*>(chrMemory.data()), chrMemory.size());
    }

    valid = true;
}

uint8_t Cartridge::read(uint16_t addr, bool readonly) {
    return mapper->cpuRead(addr, readonly);
}

void Cartridge::write(uint16_t addr, uint8_t data) {
    mapper->cpuWrite(addr, data);
}

uint8_t Cartridge::ppuRead(uint16_t addr, bool readonly) {
    return mapper->ppuRead(addr, readonly);
}

void Cartridge::ppuWrite(uint16_t addr, uint8_t data) {
    mapper->ppuWrite(addr, data);
}

void Cartridge::initMapper(uint8_t id) {
    switch (id) {
        case 0:
            mapper = std::make_unique<M000>(prgBanks, chrBanks);
            break;
        case 1:
            mapper = std::make_unique<M001>(prgBanks, chrBanks);
            break;
        case 2:
            mapper = std::make_unique<M002>(prgBanks, chrBanks);
            break;
        default: return;
    }
}