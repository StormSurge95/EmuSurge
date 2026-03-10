#include "Cartridge.h"

#include <fstream>

Cartridge::Cartridge(const std::string& filename) {
    // open ROM file
    std::ifstream file(filename, std::ifstream::binary);

    // very file was found and opened
    if (!file) return;

    // read header metadata
    INESHeader header;
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

    // read prgMemory
    prgMemory.resize((size_t)prgBanks * 16384);
    file.read(reinterpret_cast<char*>(prgMemory.data()), prgMemory.size());

    if (chrBanks == 0) {
        chrMemory.resize(8192);
    } else {
        chrMemory.resize(chrBanks * 8192);
        file.read(reinterpret_cast<char*>(chrMemory.data()), chrMemory.size());
    }

    valid = true;
}

uint8_t Cartridge::read(uint16_t addr, bool readonly) {
    if (addr >= 0x8000) {
        uint32_t mapped = addr - 0x8000;

        if (prgBanks == 1)
            mapped &= 0x3FFF;

        return prgMemory[mapped];
    }

    return 0;
}

void Cartridge::write(uint16_t addr, uint8_t data) {}

bool Cartridge::ppuRead(uint16_t addr, uint8_t& data) {
    if (addr < 0x2000) {
        data = chrMemory[addr];
        return true;
    }
    return false;
}

bool Cartridge::ppuWrite(uint16_t addr, uint8_t data) {
    if (addr < 0x2000) {
        if (chrMemory.size() > 0) {
            chrMemory[addr] = data;
            return true;
        }
    }
    return false;
}