#pragma once

#include "../../core/Bus.h"

#include <array>
#include <memory>

class NES_CPU;
class NES_PPU;
class NES_Controller;
class NES_APU;
class Mapper;

class NES_Bus : public Bus {
    public:
        std::array<uint8_t, 2048> ram;
        std::shared_ptr<NES_CPU> cpu = nullptr;
        std::shared_ptr<NES_PPU> ppu = nullptr;
        std::shared_ptr<NES_APU> apu = nullptr;
        std::shared_ptr<Mapper> mapper = nullptr;
        std::shared_ptr<NES_Controller> controller1 = nullptr;
        std::shared_ptr<NES_Controller> controller2 = nullptr;

        NES_Bus();
        ~NES_Bus() = default;

        uint8_t read(uint16_t addr, bool readonly = false) override;
        void write(uint16_t addr, uint8_t data);

        void connectCPU(std::shared_ptr<NES_CPU> cpu) { this->cpu = cpu; }
        void connectPPU(std::shared_ptr<NES_PPU> ppu) { this->ppu = ppu; }
        void connectAPU(std::shared_ptr<NES_APU> apu) { this->apu = apu; }
        void connectMapper(std::shared_ptr<Mapper> mapper) { this->mapper = mapper; }
        void connectController(std::shared_ptr<NES_Controller> c, uint8_t player);
        void disconnectController(uint8_t player);
};