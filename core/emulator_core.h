#pragma once

#include "../debug/DebugState.h"

#include <cstdint>
#include <string>

class EmulatorCore {
    public:
        bool frameComplete = false;
        unsigned long long systemClockCounter = 0;
        DebugState debug;

        virtual ~EmulatorCore() = default;
        virtual bool loadROM(const std::string& path) = 0;
        virtual void reset() = 0;
        virtual void clock() = 0;
        virtual void runFrame() = 0;
        virtual void setControllerState(int controller, uint8_t state) = 0;
        virtual const uint32_t* getFrameBuffer() const = 0;
        virtual int getScreenWidth() const = 0;
        virtual int getScreenHeight() const = 0;
        virtual bool complete() = 0;
        virtual void runCycles(uint64_t cycles) = 0;

        virtual void debugHook() = 0;
        virtual void runUntilPC(uint16_t addr) = 0;
        virtual void runUntilCycle(uint64_t cycle) = 0;
};