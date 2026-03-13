#include "NES.h"
#include "NES_Bus.h"

NES::NES(bool d) {
    debug = d;

    // initialize bus & cpu
    bus = std::make_unique<NES_Bus>();
    cpu = std::make_unique<NES_CPU>();

    // connect CPU to bus
    cpu->connectBus(bus.get());
    if (debug) cpu->attachTraceStream(&traceFile);

    // init CPU RAM and map to Bus
    cpuRam = std::make_unique<RAM>(2048);
    bus->map(cpuRam.get(), 0x0000, 0x1FFF, "CPU RAM");

    // PPU and map to Bus
    ppu = std::make_unique<NES_PPU>();
    bus->map(ppu.get(), 0x2000, 0x3FFF, "PPU");
}

bool NES::loadCartridge(const std::string& path) {
    // create cartridge
    cart = std::make_shared<Cartridge>(path);

    // verify cartridge
    if (cart->isValid()) {
        // map cartridge to bus
        bus->map(cart.get(), 0x8000, 0xFFFF, "CART");
        // connect cartridge to ppu
        ppu->connectCartridge(cart.get());

        if (debug) traceFile.open("trace.log");

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
            if (bus->dmaActive) bus->clockDMA(systemClockCounter);
            else cpu->clock();
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