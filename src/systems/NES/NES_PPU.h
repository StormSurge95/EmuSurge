#pragma once

#include <array>
#include <cstdint>

#include "../../core/Device.h"
#include "Cartridge.h"

class NES_PPU : public Device {
    public:
        bool nmiRequested = false;
        bool frameComplete = false;

        NES_PPU();
        ~NES_PPU();

        void clock();
        void reset();

        uint8_t read(uint16_t addr, bool readonly = false) override;
        void write(uint16_t addr, uint8_t data) override;

        bool ppuRead(uint16_t addr, uint8_t& data);
        bool ppuWrite(uint16_t addr, uint8_t data);

        const uint32_t* getFrameBuffer() const;
        inline void connectCartridge(Cartridge* cartridge) { cart = cartridge; }

    private:
        Cartridge* cart = nullptr;

        // CPU visible registers
        struct CONTROL {
            uint8_t nametableBase = 0;
            bool incrementBy32 = false;
            bool spritePatternBase2 = false;
            bool backgroundPatternBase2 = false;
            bool spritesAre8x16 = false;
            const bool masterSlave = false;
            bool nmi_enabled = false;

            CONTROL& operator=(uint8_t val) {
                nametableBase = (uint8_t)(val & 0x03);
                incrementBy32 = !!(val & (1 << 2));
                spritePatternBase2 = !!(val & (1 << 3));
                backgroundPatternBase2 = !!(val & (1 << 4));
                spritesAre8x16 = !!(val & (1 << 5));
                nmi_enabled = !!(val & (1 << 7));
                return *this;
            }

            uint8_t value() const {
                return (
                    (+nmi_enabled << 7) |
                    (+spritesAre8x16 << 5) |
                    (+backgroundPatternBase2 << 4) |
                    (+spritePatternBase2 << 3) |
                    (+incrementBy32 << 2) |
                    nametableBase);
            }
        } PPUCTRL;
        struct MASK {
            bool greyscaleMode = false;
            bool enableBackgroundLeft = false;
            bool enableSpritesLeft = false;
            bool enableBackground = false;
            bool enableSprites = false;
            bool emphasizeRed = false;
            bool emphasizeGreen = false;
            bool emphasizeBlue = false;

            MASK& operator=(uint8_t val) {
                greyscaleMode = !!(val & (1 << 0));
                enableBackgroundLeft = !!(val & (1 << 1));
                enableSpritesLeft = !!(val & (1 << 2));
                enableBackground = !!(val & (1 << 3));
                enableSprites = !!(val & (1 << 4));
                emphasizeRed = !!(val & (1 << 5));
                emphasizeGreen = !!(val & (1 << 6));
                emphasizeBlue = !!(val & (1 << 7));
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
                    (+greyscaleMode));
            }
        } PPUMASK;
        struct STATUS {
            bool spriteOverflow = false;
            bool spriteZeroHit = false;
            bool isInVblank = false;

            STATUS& operator=(uint8_t val) {
                spriteOverflow = !!(val & (1 << 5));
                spriteZeroHit = !!(val & (1 << 6));
                isInVblank = !!(val & (1 << 7));
            }

            uint8_t value() const {
                return (
                    (+isInVblank << 7) |
                    (+spriteZeroHit << 6) |
                    (+spriteOverflow << 5));
            }
        } PPUSTATUS;
        uint8_t OAMADDR = 0x00;
        uint8_t OAMDATA = 0x00;
        uint8_t PPUSCROLL = 0x00;
        uint8_t PPUADDR = 0x00;
        uint8_t PPUDATA = 0x00;
        uint8_t OAMDMA = 0x00;

        uint16_t cycle = 0x0000;
        uint16_t scanline = 0x0000;
        uint64_t frame = 0x0000000000000000;

        std::array<uint32_t, 256 * 240> frameBuffer{ 0 };

        // scroll variables
        bool w = false; // write latch
        uint16_t vramAddr = 0;
        uint16_t v = 0; // scroll position/current VRAM address
        uint16_t t = 0; // START scroll position/next VRAM address
        uint8_t x = 0;  // fine-x position of current scroll

        std::array<uint8_t, 2048> nametables{ 0 };
        std::array<uint32_t, 32> paletteRam{ 0 };
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

        uint16_t nametableBaseAddr() const {
            if (PPUCTRL.nametableBase == 0x00)
                return 0x2000;
            else if (PPUCTRL.nametableBase == 0x01)
                return 0x2400;
            else if (PPUCTRL.nametableBase == 0x02)
                return 0x2800;
            else
                return 0x2C00;
        }
        uint16_t bgPatternTblBaseAddr() const {
            if (PPUCTRL.backgroundPatternBase2)
                return 0x1000;
            else
                return 0x0000;
        }
        uint16_t spritePatternTableBaseAddr() const {
            if (PPUCTRL.spritePatternBase2 && !PPUCTRL.spritesAre8x16)
                return 0x1000;
            else
                return 0x0000;
        }

        void renderScanline();
        void plot(uint16_t x, uint16_t y, uint32_t color);

        void onPreLine();
        void onVisibleLine();
        void onVBlankLine();

        uint8_t getTileID(uint16_t x, uint16_t y);
        uint8_t getPixel(uint8_t id, uint8_t row, uint8_t x);
        uint8_t getAttr(uint16_t x, uint16_t y);

        void renderPatternTable(uint16_t table, uint16_t screenX, uint16_t screenY);

        uint16_t ntbase = 0;
        uint16_t bptbase = 0;
};