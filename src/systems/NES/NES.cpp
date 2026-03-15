#include "NES.h"
#include "NES_Bus.h"

#include <array>
#include <memory>

NES::NES(bool d) {
    debug = d;

    // initialize bus/cpu/ppu/controllers
    bus = std::make_shared<NES_Bus>();
    cpu = std::make_shared<NES_CPU>();
    ppu = std::make_shared<NES_PPU>();
    controller1 = std::make_shared<NES_Controller>(1);
    controller2 = std::make_shared<NES_Controller>(2);
    controller1->other = controller2;
    controller2->other = controller1;

    // connect CPU to bus
    cpu->connectBus(bus);
    bus->connectCPU(cpu);
    if (debug) cpu->enableDebug();

    // connect PPU to Bus
    bus->connectPPU(ppu);

    // connect controllers to bus
    bus->connectController(controller1, 1);
    bus->connectController(controller2, 2);
}

bool NES::loadCartridge(const std::string& path) {
    // create cartridge
    cart = std::make_shared<Cartridge>(path);

    // verify cartridge
    if (cart->isValid()) {
        // map cartridge to bus
        bus->connectCartridge(cart);
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
            if (bus->dmaActive) bus->clockDMA(systemClockCounter);
            else {
                cart->clock();
                cpu->clock();
            }
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

void NES::update(uint8_t player, std::array<bool, 8> buttons) {
    std::shared_ptr<NES_Controller> c;
    if (player == 1) {
        for (int x = 0; x < 8; x++) {
            this->controller1->update((NES_BUTTONS)x, buttons[x]);
        }
    } else if (player == 2) {
        for (int x = 0; x < 8; x++) {
            this->controller2->update((NES_BUTTONS)x, buttons[x]);
        }
    }
}