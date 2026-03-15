#pragma once

#include "../../core/IConsole.h"
#include "../../core/RAM.h"
#include "Cartridge.h"
#include "NES_Bus.h"
#include "NES_Controller.h"
#include "NES_CPU.h"
#include "NES_PPU.h"

#include <memory>

class NES : public IConsole {
    public:
        std::shared_ptr<NES_CPU> cpu;
        bool debug = false;

        NES(bool debug = false);

        void reset() override;
        void clock() override;

        bool loadCartridge(const std::string& path) override;

        const uint32_t* getFrameBuffer() const override;
        int getScreenWidth() const override { return 256; }
        int getScreenHeight() const override { return 240; }

    private:
        std::shared_ptr<NES_Bus> bus;
        std::shared_ptr<Cartridge> cart;
        std::shared_ptr<RAM> cpuRam;
        std::shared_ptr<NES_PPU> ppu;
        std::shared_ptr<NES_Controller> controller1;
        std::shared_ptr<NES_Controller> controller2;
        uint64_t systemClockCounter = 0;
};