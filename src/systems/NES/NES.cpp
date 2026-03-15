#include "NES.h"
#include "NES_Bus.h"

#include <memory>

NES::NES(bool d) {
    debug = d;

    // initialize bus & cpu
    bus = std::make_shared<NES_Bus>();
    cpu = std::make_shared<NES_CPU>();

    // connect CPU to bus
    cpu->connectBus(bus);
    bus->connectCPU(cpu);
    if (debug) cpu->enableDebug();

    // PPU and map to Bus
    ppu = std::make_shared<NES_PPU>();
    bus->connectPPU(ppu);
}

bool NES::loadCartridge(const std::string& path) {
    // create cartridge
    cart = std::make_shared<Cartridge>(path);

    // verify cartridge
    if (cart->isValid()) {
        // map cartridge to bus
        bus->connectMapper(cart->mapper);
        // connect cartridge to ppu
        ppu->connectCartridge(cart);

        cpu->reset();

        return true;
    }
    return false;
}

void NES::reset() {
    cpu->reset();
    ppu->reset();
    // reset apu
}

void NES::clock() {
    do {
        ppu.get()->clock();

        if (systemClockCounter % 3 == 0) {
            //if (bus->dmaActive) bus->clockDMA(systemClockCounter);
            //else {
                cart->clock();
                cpu->clock();
            //}
        }

        if (ppu.get()->nmiRequested) {
            cpu->NMI();
            ppu->nmiRequested = false;
        }

        systemClockCounter++;
    } while (!ppu->frameComplete);

    ppu->frameComplete = false;
}

const uint32_t* NES::getFrameBuffer() const {
    return ppu->getFrameBuffer();
}