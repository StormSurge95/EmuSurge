#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "../../core/Device.h"
#include "Cartridge.h"
#include "NES_CPU.h"

class NES_PPU : public Device {
    public:
        bool nmiOutput = false;
        bool nmiOutputPrev = false;
        bool nmiRequested = false;
        bool frameComplete = false;
        uint16_t scanline = 0;
        uint16_t cycle = 0;
        std::array<uint8_t, 256> primaryOAM{ 0 };

        NES_PPU();
        ~NES_PPU() = default;

        void clock();
        void reset() {}

        uint8_t read(uint16_t addr, bool readonly = false) override;
        void write(uint16_t addr, uint8_t data) override;

        uint8_t ppuRead(uint16_t addr, bool readonly = false);
        void ppuWrite(uint16_t addr, uint8_t data);

        const uint32_t* getFrameBuffer() const { return frameBuffer.data(); }
        void connectCartridge(std::shared_ptr<Cartridge> cart) { this->cart = cart; }

        bool oddFrame = false;
        std::shared_ptr<Cartridge> cart = nullptr;
        std::shared_ptr<NES_CPU> cpu = nullptr;
        std::array<uint32_t, 61440> frameBuffer{ 0 };
        std::array<uint32_t, 64> masterPalette = {
            0xFF666666,0xFF002A88,0xFF1412A7,0xFF3B00A4,0xFF5C007E,0xFF6E0040,0xFF6C0600,0xFF561D00,
            0xFF333500,0xFF0B4800,0xFF005200,0xFF004F08,0xFF00404D,0xFF000000,0xFF000000,0xFF000000,
            0xFFADADAD,0xFF155FD9,0xFF4240FF,0xFF7527FE,0xFFA01ACC,0xFFB71E7B,0xFFB53120,0xFF994E00,
            0xFF6B6D00,0xFF388700,0xFF0E9300,0xFF008F32,0xFF007C8D,0xFF000000,0xFF000000,0xFF000000,
            0xFFFFFEFF,0xFF64B0FF,0xFF9290FF,0xFFC676FF,0xFFF36AFF,0xFFFE6ECC,0xFFFE8170,0xFFEA9E22,
            0xFFBCBE00,0xFF88D800,0xFF5CE430,0xFF45E082,0xFF48CDDE,0xFF4F4F4F,0xFF000000,0xFF000000,
            0xFFFFFEFF,0xFFC0DFFF,0xFFD3D2FF,0xFFE8C8FF,0xFFFBC2FF,0xFFFEC4EA,0xFFFECCC5,0xFFF7D8A5,
            0xFFE4E594,0xFFCFEF96,0xFFBDF4AB,0xFFB3F3CC,0xFFB5EBF2,0xFFB8B8B8,0xFF000000,0xFF000000
        };
        std::array<uint8_t, 4096> nametables{ 0 };
        std::array<uint8_t, 32> palettes{ 0 };

        // REGISTERS
        struct control {
            uint8_t nametableBase = 0;
            bool incrementBy32 = false;
            bool spritePatternBase = false;
            bool backgroundPatternBase = false;
            bool spritesAre8x16 = false;
            bool nmiEnabled = false;

            control(uint8_t value) {
                this->nametableBase = (uint8_t(value & 0x03));
                this->incrementBy32 = !!(value & (1 << 2));
                this->spritePatternBase = !!(value & (1 << 3));
                this->backgroundPatternBase = !!(value & (1 << 4));
                this->spritesAre8x16 = !!(value & (1 << 5));
                this->nmiEnabled = !!(value & 1 << 7);
            }

            control& operator=(uint8_t value) {
                this->nametableBase = (uint8_t(value & 0x03));
                this->incrementBy32 = !!(value & (1 << 2));
                this->spritePatternBase = !!(value & (1 << 3));
                this->backgroundPatternBase = !!(value & (1 << 4));
                this->spritesAre8x16 = !!(value & (1 << 5));
                this->nmiEnabled = !!(value & 1 << 7);
                return *this;
            }

            uint8_t value() const {
                return (
                    (+this->nmiEnabled << 7) |
                    (+this->spritesAre8x16 << 5) |
                    (+this->backgroundPatternBase << 4) |
                    (+this->spritePatternBase << 3) |
                    (+this->incrementBy32 << 2) |
                    this->nametableBase
                );
            }

            uint16_t backgroundPatternSelect() const { return (this->backgroundPatternBase ? 0x1000 : 0x0000); }
            uint16_t spritePatternSelect() const { return (this->spritePatternBase ? 0x1000 : 0x0000); }
            uint8_t incrementMode() const { return (this->incrementBy32 ? 32 : 1); }
            uint16_t nametableSelect() const {
                switch (this->nametableBase) {
                    case 0x00:
                        return 0x2000;
                    case 0x01:
                        return 0x2400;
                    case 0x02:
                        return 0x2800;
                    case 0x03:
                        return 0x2C00;
                }
            }
        } PPUCTRL;
        struct mask {
            bool greyscaleMode = false;
            bool enableBackgroundLeft = false;
            bool enableSpritesLeft = false;
            bool enableBackground = false;
            bool enableSprites = false;
            bool emphasizeRed = false;
            bool emphasizeGreen = false;
            bool emphasizeBlue = false;

            mask(uint8_t value) {
                this->greyscaleMode =           !!(value & (1 << 0));
                this->enableBackgroundLeft =    !!(value & (1 << 1));
                this->enableSpritesLeft =       !!(value & (1 << 2));
                this->enableBackground =        !!(value & (1 << 3));
                this->enableSprites =           !!(value & (1 << 4));
                this->emphasizeRed =            !!(value & (1 << 5));
                this->emphasizeGreen =          !!(value & (1 << 6));
                this->emphasizeBlue =           !!(value & (1 << 7));
            }

            mask& operator=(uint8_t value) {
                this->greyscaleMode = !!(value & (1 << 0));
                this->enableBackgroundLeft = !!(value & (1 << 1));
                this->enableSpritesLeft = !!(value & (1 << 2));
                this->enableBackground = !!(value & (1 << 3));
                this->enableSprites = !!(value & (1 << 4));
                this->emphasizeRed = !!(value & (1 << 5));
                this->emphasizeGreen = !!(value & (1 << 6));
                this->emphasizeBlue = !!(value & (1 << 7));
                return *this;
            }
            uint8_t value() const {
                return (
                    (+emphasizeBlue << 7) |
                    (+emphasizeGreen << 6) |
                    (+emphasizeRed << 5) |
                    (+enableSprites << 4) |
                    (+enableBackground << 3) |
                    (+enableSpritesLeft << 2) |
                    (+enableBackgroundLeft << 1) |
                    (+greyscaleMode)
                );
            }
        } PPUMASK;
        struct status {
            bool spriteOverflow = false;
            bool spriteZeroHit = false;
            bool isInVblank = false;
            uint8_t openBus = 0;

            status() = default;
            status(uint8_t value) {
                this->isInVblank = !!(value & (1 << 7));
                this->spriteZeroHit = !!(value & (1 << 6));
                this->spriteOverflow = !!(value & (1 << 5));
                this->openBus = (value & ((1 << 5) - 1));
            }

            status& operator=(uint8_t value) {
                this->isInVblank = !!(value & (1 << 7));
                this->spriteZeroHit = !!(value & (1 << 6));
                this->spriteOverflow = !!(value & (1 << 5));
                this->openBus = (value & ((1 << 5) - 1));
                return *this;
            }

            uint8_t value() const {
                return (
                    (+this->isInVblank << 7) |
                    (+this->spriteZeroHit << 6) |
                    (+this->spriteOverflow << 5) |
                    this->openBus
                );
            }
        } PPUSTATUS = 0xA0;
        uint8_t OAMADDR = 0;
        uint8_t PPUSCROLL = 0;
        uint8_t PPUADDR = 0;
        uint8_t PPUDATA = 0;
        uint8_t OAMDMA = 0;
        uint8_t dataBuffer = 0;
        uint8_t ppuBus = 0x00;

        uint16_t v = 0;  // during rendering, used for scroll position; outside rendering, used as current VRAM address
        uint16_t t = 0;  // during rendering, specifies starting coarse-x scroll for next scanline and starting y scroll for screen; outside rendering, holds scroll or VRAM before transferring it to v
        uint8_t x = 0;  // fine-x position of current scroll, used during rendering alongside v
        bool w = false; // write-latch for PPUSCROLL/PPUADDR; clears on read os PPUSTATUS

        inline bool rendering() const { return (this->PPUMASK.enableBackground || this->PPUMASK.enableSprites); }
        inline void triggerNMI() { this->nmiRequested = true; }

        uint8_t nextNametableByte = 0x00;
        uint8_t nextAttributeByte = 0x00;
        uint8_t nextPatternByteLo = 0x00;
        uint8_t nextPatternByteHi = 0x00;

        uint16_t patternShiftLo = 0x0000;
        uint16_t patternShiftHi = 0x0000;
        uint16_t attributeShiftLo = 0x0000;
        uint16_t attributeShiftHi = 0x0000;

        inline void copyHorizontalBits() { this->v = (this->v & ~0b0000010000011111) | (this->t & 0b0000010000011111); }
        inline void copyVerticalBits()   { this->v = (this->v & ~0b0111101111100000) | (this->t & 0b0111101111100000); }
        inline uint8_t coarseX() const { return this->v & 0x1F; }
        inline uint8_t coarseY() const { return (this->v >> 5) & 0x1F; }
        inline uint8_t fineY() const { return (this->v >> 12) & 0x07; }

        void incrementX();
        void incrementY();
        void shiftBackground();
        void loadBackgroundShifters();
        void fetchNametableByte();
        void fetchAttributeByte();
        void fetchPatternLo();
        void fetchPatternHi();

        void renderPixel();
};