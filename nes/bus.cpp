#include "bus.h"
#include "cartridge.h"

#include <cstdint>
#include <memory>

Bus::Bus() : cpuRam{ 0 } {}

void Bus::insertCartridge(const std::shared_ptr<Cartridge>& cartridge) {
    cart = cartridge;
}

uint8_t Bus::cpuRead(uint16_t addr, bool readOnly)
{
    uint8_t data = 0x00;

    if (addr <= 0x1FFF) {
        data = cpuRam[addr & 0x07FF];
    } else if (addr >= 0x2000 && addr <= 0x3FFF) {
        // TODO
    } else if (addr >= 0x8000)
        cart->cpuRead(addr, data);

    return data;
}

void Bus::cpuWrite(uint16_t addr, uint8_t data) {
    if (cart->cpuWrite(addr, data)) {
        // cartridge handled it
    } else if (addr >= 0x0000 && addr <= 0x1FFF)
        cpuRam[addr & 0x07FF] = data;
}

void Bus::reset() {}