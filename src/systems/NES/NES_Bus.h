#pragma once

#include "../../core/Bus.h"
#include "NES_CPU.h"
#include "NES_PPU.h"

class NES_Bus : public Bus {
    public:
        bool dmaActive = false;
        uint8_t dmaPage = 0x00;
        uint8_t dmaAddr = 0x00;
        bool dmaDummy = true;

        inline void clockDMA(const uint64_t masterCycle) {
            if (dmaDummy) {
                dmaDummy = false;
                if (masterCycle % 1) return;
            }

            if (dmaAddr & 0x01) // only perform writes on even cycles (keeps cycle accuracy)
                return;

            // hold devices for convenience
            NES_CPU* cpu = dynamic_cast<NES_CPU*>(mappings.find("CPU")->second.device);
            NES_PPU* ppu = dynamic_cast<NES_PPU*>(mappings.find("PPU")->second.device);
            // read bytes from cpu
            uint8_t b1 = cpu->read(((uint16_t)dmaPage << 8) | dmaAddr);
            uint8_t b2 = cpu->read(((uint16_t)dmaPage << 8) | (dmaAddr + 1));
            // write byte to ppu
            ppu->writeDMAByte(b1);
            dmaActive = !ppu->writeDMAByte(b2);
        }

        inline void write(uint16_t addr, uint8_t data) {
            if (addr == 0x4014) {
                dmaActive = true;
                dmaPage = data;
                dynamic_cast<NES_PPU*>(mappings.find("PPU")->second.device)->write(0x2003, 0x00);
            } else Bus::write(addr, data);
        }
};