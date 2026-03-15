#include "NES_Bus.h"
#include "NES_Controller.h"
#include "Mappers/Mapper.h"

#include <memory>

NES_Bus::NES_Bus() : ram{} {}

uint8_t NES_Bus::read(uint16_t addr, bool readonly) {
    if (addr >= 0x0000 && addr <= 0x07FF) {
        return this->ram[addr];
    } else if (addr >= 0x0800 && addr <= 0x1FFF) {
        return this->read(0x0000 + (addr - 0x0800) % 0x0800);
    } else if (addr >= 0x2000 && addr <= 0x2007) {
        // PPU registers
    } else if (addr >= 0x2008 && addr <= 0x3FFF) {
        return this->read(0x2000 + (addr - 0x2008) % 0x0008);
    } else if (addr >= 0x4000 && addr <= 0x4015) {
        // APU registers
        // PPU's OAMDMA register
        // APU Status register
    } else if (addr == 0x4016) {
        return this->controller1->onRead();
    } else if (addr == 0x4017) {
        return this->controller2->onRead();
    } else if (addr >= 0x4020 && addr <= 0xFFFF) {
        return this->mapper->cpuRead(addr);
    }

    return 0x00;
}

void NES_Bus::write(uint16_t addr, uint8_t data) {
    if (addr >= 0x0000 && addr <= 0x07FF) {
        this->ram[addr] = data;
    } else if (addr >= 0x0800 && addr <= 0x1FFF) {
        return this->write(0x0000 + (addr - 0x0800) % 0x0800, data);
    } else if (addr >= 0x2000 && addr <= 0x2007) {
        // PPU Registers
    } else if (addr >= 0x2008 && addr <= 0x3FFF) {
        return this->write(0x2000 + (addr - 0x2008) % 0x2008, data);
    } else if (addr >= 0x4000 && addr <= 0x4015) {
        // APU Registers
        // PPU's OAMDMA register
        // APU Status register
    } else if (addr == 0x4016) {
        return this->controller1->onWrite(data);
    } else if (addr >= 4017 && addr <= 4019) {
        // APU Frame Counter
    } else if (addr >= 0x4020 && addr <= 0xFFFF) {
        return this->mapper->cpuWrite(addr, data);
    }
}

void NES_Bus::connectController(std::shared_ptr<NES_Controller> c, uint8_t player) {
    if (player == 1) {
        this->controller1 = c;
    } else if (player == 2) {
        this->controller2 = c;
        this->controller1->other = this->controller2;
        this->controller2->other = this->controller1;
    }
}

void NES_Bus::disconnectController(uint8_t player) {
    if (player == 1) {
        if (this->controller2 != nullptr) {
            // get strobe from current player 1
            bool s = this->controller1->strobe;
            // switch player 2 to player 1
            this->controller1 = this->controller2;
            // assign previous strobe to new player 1
            this->controller1->strobe = s;
            // clear player 1's reference to player 2
            this->controller1->other = nullptr;
        } else
            this->controller1 = nullptr;
    } else if (player == 2) {
        this->controller2 = nullptr;
    }
}