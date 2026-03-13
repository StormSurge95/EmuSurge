#pragma once

#include "../../core/Bus.h"
#include "NES_CPU.h"
#include "NES_PPU.h"

class NES_Bus : public Bus {
    public:
        bool dmaActive = false;
        uint8_t dmaPage = 0x00;
        uint8_t dmaAddr = 0x00;
        uint8_t dmaData = 0x00;
        bool dmaDummy = true;

        inline void clockDMA(const uint64_t masterCycle) {
            if (dmaDummy) {
                if (masterCycle & 0x01)
                    dmaDummy = false;
                return;
            }

            // hold devices for convenience
            NES_CPU* cpu = dynamic_cast<NES_CPU*>(mappings.find("CPU")->second.device);
            NES_PPU* ppu = dynamic_cast<NES_PPU*>(mappings.find("PPU")->second.device);
            
            if ((masterCycle & 0x01) == 0) {
                dmaData = cpu->read(((uint16_t)dmaPage << 8) | dmaAddr);
            } else {
                ppu->writeDMAByte(dmaData);
                dmaAddr++;

                if (dmaAddr == 0x00) {
                    dmaActive = false;
                    dmaDummy = true;
                }
            }
        }

        inline void write(uint16_t addr, uint8_t data) {
            if (addr == 0x4014) {
                dmaActive = true;
                dmaPage = data;
                dmaAddr = 0x00;
                dmaDummy = true;
                dynamic_cast<NES_PPU*>(mappings.find("PPU")->second.device)->write(0x2003, 0x00);
            } else Bus::write(addr, data);
        }
};