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
        uint16_t v = 0; // scroll position/current VRAM address
        uint16_t t = 0; // START scroll position/next VRAM address
        uint8_t x = 0;  // fine-x position of current scroll

        std::array<uint8_t, 2048> nametables;

        void renderScanline();
        void plot(uint16_t x, uint16_t y, uint32_t color);

        void onPreLine();
        void onVisibleLine();
        void onVBlankLine();
};