#pragma once

#include "../core/emulator_core.h"

#include <memory>
#include <string>

class EmulatorManager {
    public:
        bool loadNES(const std::string& path);

        void run();

        void stepInstruction();

        void runFrame();

        void renderFrame();

        const unsigned int* getFrameBuffer() const;

        int getWidth() const;
        int getHeight() const;

    private:
        std::unique_ptr<EmulatorCore> core;
        bool running = false;
};