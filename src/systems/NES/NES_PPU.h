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
        bool oddFrame = false;

        // CPU visible registers
        struct CONTROL {
            uint8_t nametableBase = 0;              // bits 0-1
            bool incrementBy32 = false;             // bit 2
            bool spritePatternBase2 = false;        // bit 3
            bool backgroundPatternBase2 = false;    // bit 4
            bool spritesAre8x16 = false;            // bit 5
            const bool masterSlave = false;         // bit 6
            bool nmi_enabled = false;               // bit 7

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
        uint8_t fine_x = 0;  // fine-x position of current scroll

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

        uint16_t val = 0x03C0;

        uint16_t ntbase = 0;
        uint16_t bptbase = 0;


        inline void triggerNMI() { nmiRequested = true; }

        // tile latches
        uint8_t nextNametableByte = 0x00;
        uint8_t nextAttributeByte = 0x00;
        uint8_t nextPatternLowByte = 0x00;
        uint8_t nextPatternHighByte = 0x00;

        // background shift registers
        uint16_t pattShiftLo = 0x0000;
        uint16_t pattShiftHi = 0x0000;
        uint16_t attrShiftLo = 0x0000;
        uint16_t attrShiftHi = 0x0000;

        uint8_t coarseX() const {
            return v & 0x1F;
        }
        uint8_t coarseY() const {
            return (v >> 5) & 0x1F;
        }
        uint8_t fineY() const {
            return (v >> 12) & 0x07;
        }

        inline void shiftBackground() {
            if (PPUMASK.enableBackground) {
                pattShiftLo <<= 1;
                pattShiftHi <<= 1;

                attrShiftLo <<= 1;
                attrShiftHi <<= 1;
            }
        }
        inline void loadBackgroundShifters() {
            pattShiftLo = (pattShiftLo & 0xFF00) | nextPatternLowByte;
            pattShiftHi = (pattShiftHi & 0xFF00) | nextPatternHighByte;

            attrShiftLo = (attrShiftLo & 0xFF00) | ((nextAttributeByte & 0b01) ? 0xFF : 0x00);
            attrShiftHi = (attrShiftHi & 0xFF00) | ((nextAttributeByte & 0b10) ? 0xFF : 0x00);
        }
        inline void fetchNametableByte() {
            uint16_t addr = nametableBaseAddr() | (v & 0x0FFF);
            ppuRead(addr, nextNametableByte);
        }
        inline void fetchAttributeByte() {
            uint16_t addr = (nametableBaseAddr() + 0x03C0) |
                (v & 0x0C00) |
                ((coarseY() >> 2) << 3) |
                (coarseX() >> 2);
            ppuRead(addr, nextAttributeByte);

            if (coarseY() & 0x02) nextAttributeByte >>= 4;
            if (coarseX() & 0x02) nextAttributeByte >>= 2;

            nextAttributeByte &= 0x03;
        }
        inline void fetchPatternLow() {
            uint16_t addr = bgPatternTblBaseAddr() |
                (nextNametableByte << 4) |
                fineY();
            ppuRead(addr, nextPatternLowByte);
        }
        inline void fetchPatternHigh() {
            uint16_t addr = bgPatternTblBaseAddr() |
                (nextNametableByte << 4) |
                fineY();
            ppuRead(addr + 8, nextPatternHighByte);
        }
        inline void incrementX() {
            if ((v & 0x001F) == 31) {
                v &= ~0x001F;
                v ^= 0x0400;
            } else {
                v++;
            }
        }
        inline void incrementY() {
            if ((v & 0x7000) != 0x7000) {
                v += 0x1000;
            } else {
                v &= ~0x7000;
                int8_t y = (v & 0x03E0) >> 5;
                if (y == 29) {
                    y = 0;
                    v ^= 0x0800;
                } else if (y == 31) {
                    y = 0;
                } else {
                    y++;
                }
                v = (v & ~0x03E0) | (y << 5);
            }
        }
        inline void copyHorizontalBits() { v = (v & ~0x041F) | (t & 0x041F); }
        inline void copyVerticalBits() { v = (v & ~0x73E0) | (t & 0x73E0); }
        void renderPixel();

        struct SPRITE {
            uint8_t yCoord = 0x00;
            uint8_t tileIndex = 0x00;
            struct ATTR {
                bool verticalFlip = false;
                bool horizontalFlip = false;
                bool priority = true;
                uint8_t palette = 0x00;

                ATTR& operator=(uint8_t b) {
                    verticalFlip = !!(b & 0b10000000);
                    horizontalFlip = !!(b & 0b01000000);
                    priority = !(b & 0b00100000);
                    palette = (b & 0x00001100) >> 2;
                }
            } attr{};
            uint8_t xCoord = 0x00;
        };
        uint8_t primaryOAM[256]{ 0 };
        void evaluateSprites();
};