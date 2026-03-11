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

        #pragma region PPUSTATUS READ/WRITE
        inline bool isInVblank() const { return !!(PPUSTATUS & 0x80); }
        inline void isInVblank(bool set) {
            if (set) PPUSTATUS |= 0x80;
            else PPUSTATUS &= ~0x80;
        }
        inline bool isSprite0Hit() const { return !!(PPUSTATUS & 0x40); }
        inline void isSprite0Hit(bool set) {
            if (set) PPUSTATUS |= 0x40;
            else PPUSTATUS &= ~0x40;
        }
        inline bool isSpriteOverflow() const { return !!((PPUSTATUS & 0x20)); }
        inline void isSpriteOverflow(bool set) {
            if (set) PPUSTATUS |= 0x20;
            else PPUSTATUS &= ~0x20;
        }
        #pragma endregion
        #pragma region PPUCTRL READ/WRITE
        inline bool nmiEnabled() const { return !!(PPUCTRL & 0x80); }
        inline void nmiEnabled(bool set) {
            if (set) PPUCTRL |= 0x80;
            else PPUCTRL &= ~0x80;
        }
        inline bool isSprite8x16() const { return !!(PPUCTRL & 0x20); }
        inline bool isSprite8x16(bool set) {
            if (set) PPUCTRL |= 0x20;
            else PPUCTRL &= ~0x20;
        }
        inline uint16_t bgPatternTblBaseAddr() const {
            if (PPUCTRL & 0x10) return 0x1000;
            return 0x0000;
        }
        inline void bgPatterlTblBaseAddr(bool set) {
            if (set) PPUCTRL |= 0x10;
            else PPUCTRL &= ~0x10;
        }
        inline uint16_t spritePatternTblBaseAddr() const {
            if (PPUCTRL & 0x08) return 0x1000;
            return 0x0000;
        }
        inline void spritePatternTblBaseAddr(bool set) {
            if (set) PPUCTRL |= 0x08;
            else PPUCTRL &= ~0x08;
        }
        inline uint8_t incrementMode() const {
            if (PPUCTRL & 0x04) return 32;
            else return 1;
        }
        inline void incrementMode(bool set) {
            if (set) PPUCTRL |= 0x04;
            else PPUCTRL &= ~0x04;
        }
        inline uint16_t nametableBaseAddr() const {
            uint16_t o = (PPUCTRL & 0x03) * 0x0400;
            return 0x2000 + o;
        }
        inline void nametableBaseAddr(uint8_t set) {
            PPUCTRL |= (set & 0x03);
        }
        #pragma endregion
        #pragma region PPUMASK READ/WRITE
        inline bool focusBlue() const { return !!(PPUMASK & 0x80); }
        inline void focusBlue(bool set) {
            if (set) PPUMASK |= 0x80;
            else PPUMASK &= ~0x80;
        }
        inline bool focusGreen() const { return !!(PPUMASK & 0x40); }
        inline void focusGreen(bool set) {
            if (set) PPUMASK |= 0x40;
            else PPUMASK &= ~0x40;
        }
        inline bool focusRed() const { return !!(PPUMASK & 0x20); }
        inline void focusRed(bool set) {
            if (set) PPUMASK |= 0x20;
            else PPUMASK &= ~0x20;
        }
        inline bool enableSprites() const { return !!(PPUMASK & 0x10); }
        inline void enableSprites(bool set) {
            if (set) PPUMASK |= 0x10;
            else PPUMASK &= ~0x10;
        }
        inline bool enableBackground() const { return !!(PPUMASK & 0x08); }
        inline void enableBackground(bool set) {
            if (set) PPUMASK |= 0x08;
            else PPUMASK &= ~0x08;
        }
        inline bool enableLeftColumnSprites() const { return !!(PPUMASK & 0x04); }
        inline void enableLeftColumnSprites(bool set) {
            if (set) PPUMASK |= 0x04;
            else PPUMASK &= ~0x04;
        }
        inline bool enableLeftColumnBackground() const { return !!(PPUMASK & 0x02); }
        inline void enableLeftColumnBackground(bool set) {
            if (set) PPUMASK |= 0x02;
            else PPUMASK &= ~0x02;
        }
        inline bool greyscaleEnabled() const { return !!(PPUMASK & 0x01); }
        inline void greyscaleEnabled(bool set) {
            if (set) PPUMASK |= 0x01;
            else PPUMASK &= ~0x01;
        }
        #pragma endregion

    private:
        Cartridge* cart = nullptr;

        // CPU visible registers
        uint8_t PPUCTRL = 0;
        uint8_t PPUMASK = 0;
        uint8_t PPUSTATUS = 0;
        uint8_t OAMADDR = 0;
        uint8_t OAMDATA = 0;
        uint16_t PPUSCROLL = 0;
        uint16_t PPUADDR = 0;
        uint8_t PPUDATA = 0;
        uint8_t OAMDMA = 0;

        unsigned int cycle = 0;
        int scanline = -1;
        uint32_t frame = 0;

        std::array<uint32_t, 256 * 240> frameBuffer;

        // scroll variables
        bool w = false; // write latch
        uint16_t vramAddr = 0;
        uint16_t v = 0; // scroll position/current VRAM address
        uint16_t t = 0; // START scroll position/next VRAM address
        uint8_t x = 0;  // fine-x position of current scroll

        std::array<uint8_t, 2048> nametables;
        std::array<uint32_t, 32> paletteRam = { 0 };
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

        void renderScanline();
        void plot(uint16_t x, uint16_t y, uint32_t color);

        void onPreLine();
        void onVisibleLine();
        void onVBlankLine();

        uint8_t getTileID(uint16_t x, uint16_t y);
        uint8_t getPixel(uint8_t id, uint8_t row, uint8_t x);
        uint8_t getAttr(uint16_t x, uint16_t y);
        void renderTile(uint16_t x, uint16_t y);

        void renderPatternTable(uint16_t table, uint16_t screenX, uint16_t screenY);

        uint16_t ntbase = 0;
        uint16_t bptbase = 0;
};