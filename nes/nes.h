#pragma once

#include "../core/emulator_core.h"
#include "bus.h"
#include "cartridge.h"
#include "cpu.h"

#include <cstdint>
#include <fstream>
#include <memory>
#include <string>

class NES : public EmulatorCore {
    public:
        NES();

        bool loadROM(const std::string& path) override;

        void reset() override;

        void clock() override;

        void debugHook() override;

        void runFrame() override;

        bool complete() override;

        void setControllerState(int controller, uint8_t state) override;

        const uint32_t* getFrameBuffer() const override;

        int getScreenWidth() const override;
        int getScreenHeight() const override;

        void runCycles(uint64_t cycles) override;

        inline uint64_t getSystemClock() { return systemClockCounter; }

        void runUntilPC(uint16_t addr);
        void runUntilCycle(uint64_t cycle);

    private:
        static const int WIDTH = 256;
        static const int HEIGHT = 240;
        Bus bus;
        CPU cpu;
        std::ofstream traceFile;
        std::shared_ptr<Cartridge> cart;
        uint32_t framebuffer[WIDTH * HEIGHT];
};