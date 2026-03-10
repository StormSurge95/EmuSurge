#pragma once

#include "../../core/Bus.h"
#include "../../core/IConsole.h"
#include "../../core/RAM.h"
#include "Cartridge.h"
#include "NES_CPU.h"
#include "NES_PPU.h"

#include <fstream>
#include <memory>

class NES : public IConsole {
    public:
        std::unique_ptr<NES_CPU> cpu;
        bool debug = false;

        NES(bool debug = false);

        void reset() override;
        void clock() override;

        bool loadCartridge(const std::string& path) override;

        const uint32_t* getFrameBuffer() const override;
        int getScreenWidth() const override { return 256; }
        int getScreenHeight() const override { return 240; }

    private:
        std::unique_ptr<Bus> bus;
        std::shared_ptr<Cartridge> cart;
        std::unique_ptr<RAM> cpuRam;
        std::unique_ptr<NES_PPU> ppu;
        uint64_t systemClockCounter = 0;
        std::ofstream traceFile;
        const uint32_t* frameBuffer;

        void updateFrameBuffer();
};