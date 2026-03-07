#include "../nes/nes.h"
#include "emulator_manager.h"

#include <cstdint>
#include <memory>
#include <string>

bool EmulatorManager::loadNES(const std::string& path) {
    core = std::make_unique<NES>();

    return core->loadROM(path);
}

void EmulatorManager::run() {
    running = true;

    while (running) {
        runFrame();

        renderFrame();
    }
}

void EmulatorManager::stepInstruction() {
    do {
        core->clock();
    } while (!core->complete());
}

const uint32_t* EmulatorManager::getFrameBuffer() const {
    if (core)
        return core->getFrameBuffer();

    return nullptr;
}

int EmulatorManager::getWidth() const {
    if (core)
        return core->getScreenWidth();

    return 0;
}

int EmulatorManager::getHeight() const {
    if (core)
        return core->getScreenHeight();
    
    return 0;
}

void EmulatorManager::runFrame() {
    core->runFrame();
}

void EmulatorManager::renderFrame() {
    // TODO: IMPLEMENT
}