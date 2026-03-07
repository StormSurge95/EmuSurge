#include "cartridge.h"
#include "nes.h"

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>

NES::NES() : framebuffer{ 0 } {
    cpu.connectBus(&bus);
    cpu.attachTraceStream(&traceFile);
}

bool NES::loadROM(const std::string& path) {
    cart = std::make_shared<Cartridge>(path);

    if (!cart->imageValid())
        return false;

    bus.insertCartridge(cart);

    traceFile.open("trace.log");

    reset();

    return true;
}

void NES::reset() {
    cpu.reset();
    systemClockCounter = 0;
}

void NES::clock() {
    //ppu.clock();

    if (systemClockCounter % 3 == 0)
        cpu.clock();

    if (systemClockCounter % 89342 == 0) {
        //ppu.frameComplete = false;
        frameComplete = true;
    }

    systemClockCounter++;
}

void NES::debugHook()
{
    if (!cpu.complete())
        return;

    uint16_t pc = cpu.getPC();

    if (debug.tracingEnabled)
        traceFile << cpu.trace() << std::endl;
}

void NES::runFrame()
{
    frameComplete = false;

    while (!frameComplete)
        clock();
}

bool NES::complete() {
    return cpu.complete();
}

void NES::setControllerState(int controller, uint8_t state) {
    // TODO
}

const uint32_t* NES::getFrameBuffer() const {
    return framebuffer;
}

int NES::getScreenWidth() const {
    return WIDTH;
}

int NES::getScreenHeight() const {
    return HEIGHT;
}

void NES::runCycles(uint64_t c) {
    for (uint64_t i = 0; i < c; i++)
        clock();
}

void NES::runUntilPC(uint16_t addr) {
    while (cpu.getPC() != addr)
        clock();
}

void NES::runUntilCycle(uint64_t cycle) {
    while (systemClockCounter < cycle)
        clock();
}
