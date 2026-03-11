#include "NES_PPU.h"

NES_PPU::NES_PPU() : frameBuffer{} {}

void NES_PPU::write(uint16_t addr, uint8_t data) {
    addr &= 0x0007;

    switch (addr) {
        case 0x0: // PPUCTRL
            control = data;
            t = (t & 0xF3FF) | ((uint16_t)(data & 0x03) << 10);
            break;
        case 0x1: // PPUMASK
            mask = data;
            break;
        case 0x2: // PPUSTATUS (write ignored)
            break;
        case 0x3: // OAMADDR
            oamAddr = data;
            break;
        case 0x4: // OAMDATA
            // sprite memory later
            break;
        case 0x5: // PPUSCROLL
            if (!addressLatch) {
                fineX = data & 0x07;

                t = (t & ~0x001F) | (data >> 3);

                addressLatch = true;
            } else {
                t = (t & ~0x7000) | ((data & 0x07) << 12);
                t = (t & ~0x03E0) | ((data & 0xF8) << 2);

                addressLatch = false;
            }
            break;
        case 0x6: // PPUADDR
            if (!addressLatch) {
                t = (t & 0x00FF) | ((data & 0x3F) << 8);
                addressLatch = true;
            } else {
                t = (t & 0xFF00) | data;
                t &= 0x3FFF;
                v = t;
                addressLatch = false;
            }
            break;
        case 0x7: // PPUDATA
            ppuWrite(v, data);

            if (control & 0x04)
                v += 32;
            else
                v += 1;
            break;
    }
}

uint8_t NES_PPU::read(uint16_t addr, bool readonly) {
    uint8_t data = 0x00;

    addr &= 0x0007;

    switch (addr) {
        case 0x2: // PPUSTATUS
            data = status;

            // clear VBlank
            status &= ~0x80;
            addressLatch = false;

            return data;
        case 0x4: // OAMDATA
            return data;
        case 0x7: // PPUDATA
            data = dataBuffer;

            dataBuffer = ppuRead(v);

            if (v >= 0x3F00) {
                data = ppuRead(v);
                dataBuffer = ppuRead(v - 0x1000);
            }

            uint16_t increment = (control & 0x04) ? 32 : 1;
            v += increment;
            return data;
    }
}

void NES_PPU::plot(size_t x, size_t y, uint32_t color) {
    frameBuffer[y * SCREEN_WIDTH + x] = color;
}

void NES_PPU::clock() {
    if (scanline >= 0 && scanline < 240) {
        if (cycle >= 1 && cycle <= 256) {
            // generate background pixel

            // use fine X scroll to determine which bit we need.
            uint16_t bitMux = 0x8000 >> fineX;

            // collect pattern bits to determine tile pixel
            uint8_t p0 = (bgPatternShifterLo & bitMux) > 0;
            uint8_t p1 = (bgPatternShifterHi & bitMux) > 0;
            uint8_t pixel = (p1 << 1) | p0;

            // collect palette bits to determine palette index
            uint8_t a0 = (bgAttrShifterLo & bitMux) > 0;
            uint8_t a1 = (bgAttrShifterHi & bitMux) > 0;
            uint8_t palette = (a1 << 1) | a0;

            uint32_t color = getColorFromPalette(palette, pixel);
            plot((size_t)cycle - 1, (size_t)scanline, color);
        }

        // background pipeline
        if ((cycle >= 1 && cycle <= 256) || (cycle >= 321 && cycle <= 336)) {
            updateShifters();

            uint16_t phase = (cycle - 1) % 8;

            switch (phase) {
                case 0: // fetch tile ID
                    nextTileId = ppuRead(0x2000 | (v & 0x0FFF));
                    break;
                case 2: // fetch attribute byte
                    {
                        uint16_t addr =
                            0x23C0 |
                            (v & 0x0C00) |
                            ((v >> 4) & 0x38) |
                            ((v >> 2) & 0x07);
                        uint8_t attr = ppuRead(addr);
                        if (v & 0x40) attr >>= 4;
                        if (v & 0x02) attr >>= 2;
                        nextTileAttr = attr & 0x03;
                    }
                    break;
                case 4: // fetch pattern low byte
                    {
                        uint16_t patternAddr =
                            ((control & 0x10) << 8) +
                            (nextTileId << 4) +
                            ((v >> 12) & 7);
                        nextTileLsb = ppuRead(patternAddr);
                    }
                    break;
                case 6: // fetch pattern high byte
                    {
                        uint16_t patternAddr =
                            ((control & 0x10) << 8) +
                            (nextTileId << 4) +
                            ((v >> 12) & 7);
                        nextTileMsb = ppuRead(patternAddr + 8);
                    }
                    break;
                case 7: // load shifters and increment scroll
                    loadBackgroundShifters();
                    incrementScrollX();
                    break;
            }
        }

        if (cycle == 256) incrementScrollY();

        if (cycle == 257) transferAddressX();
    }

    // Pre-render scanline
    if (scanline == -1) {
        if (cycle == 1)
            status &= ~0x80; // clear vblank
        else if (cycle >= 280 && cycle <= 304)
            transferAddressY();
    }

    // advance timing
    cycle++;

    if (cycle >= 341) {
        cycle = 0;
        scanline++;

        if (scanline == 241) {
            printf("Entered VBlank...\n");
            status |= 0x80;
            if (control & 0x80) {
                printf("NMI requested...\n");
                nmi = true;
            }
        }

        if (scanline >= 262) {
            scanline = -1;
            frameComplete = true;
        }
    }
}

uint8_t NES_PPU::ppuRead(uint16_t addr) {
    uint8_t data = 0;
    addr &= 0x3FFF;

    if (cart->ppuRead(addr, data)) {
        return data;
    } else if (addr >= 0x2000 && addr <= 0x3EFF) {
        addr &= 0x0FFF;
        if (cart->getMirror() == Cartridge::VERTICAL) {
            uint16_t A = addr & 0x03FF;
            uint16_t B = addr & 0x03FF | 0x0400;
            if (addr <= 0x03FF) data = tblName[A];
            else if (addr <= 0x07FF) data = tblName[B];
            else if (addr <= 0x0BFF) data = tblName[A];
            else data = tblName[B];
        } else {
            uint16_t A = addr & 0x03FF;
            uint16_t B = addr & 0x03FF | 0x0400;
            if (addr <= 0x03FF) data = tblName[A];
            else if (addr <= 0x07FF) data = tblName[A];
            else if (addr <= 0x0BFF) data = tblName[B];
            else data = tblName[B];

        }
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        addr &= 0x001F;
        if (addr == 0x10) addr = 0x00;
        else if (addr == 0x14) addr = 0x04;
        else if (addr == 0x18) addr = 0x08;
        else if (addr == 0x1C) addr = 0x0C;
        data = tblPalette[addr];
    }

    return data;
}

void NES_PPU::ppuWrite(uint16_t addr, uint8_t data) {
    addr &= 0x3FFF;

    if (cart->ppuWrite(addr, data)) {
        // Cartridge handled it
    } else if (addr >= 0x2000 && addr <= 0x3EFF) {
        addr &= 0x0FFF;

        uint16_t A = addr & 0x03FF;
        uint16_t B = addr & 0x03FF | 0x0400;
        if (cart->getMirror() == Cartridge::VERTICAL) {
            if (addr <= 0x03FF) tblName[A] = data;
            else if (addr <= 0x07FF) tblName[B] = data;
            else if (addr <= 0x0BFF) tblName[A] = data;
            else tblName[B] = data;
        } else {
            if (addr <= 0x03FF) tblName[A] = data;
            else if (addr <= 0x07FF) tblName[A] = data;
            else if (addr <= 0x0BFF) tblName[B] = data;
            else tblName[B] = data;
        }
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        addr &= 0x001F;
        if (addr == 0x10) addr = 0x00;
        else if (addr == 0x14) addr = 0x04;
        else if (addr == 0x18) addr = 0x08;
        else if (addr == 0x1C) addr = 0x0C;
        tblPalette[addr] = data;
    }
}

void NES_PPU::incrementScrollX() {
    if ((v & 0x001F) == 31) {
        v &= ~0x001F;   // coarseX = 0
        v ^= 0x0400;    // toggle nametable X
    } else
        v += 1;
}

void NES_PPU::incrementScrollY() {
    if ((v & 0x7000) != 0x7000)
        v += 0x1000; // increment fineY
    else {
        v &= ~0x7000; // fineY = 0

        uint16_t y = (v & 0x03E0) >> 5;

        if (y == 29) {
            y = 0;
            v ^= 0x0800; // switch vertical nametable
        } else if (y == 31)
            y = 0; // attribute table rows
        else
            y++;

        v = (v & ~0x03E0) | (y << 5);
    }
}

void NES_PPU::transferAddressX() {
    v = (v & ~0x041F) | (t & 0x041F);
}

void NES_PPU::transferAddressY() {
    v = (v & ~0x7BE0) | (t & 0x7BE0);
}

void NES_PPU::updateShifters() {
    bgPatternShifterLo <<= 1;
    bgPatternShifterHi <<= 1;

    bgAttrShifterLo <<= 1;
    bgAttrShifterHi <<= 1;
}

void NES_PPU::loadBackgroundShifters() {
    bgPatternShifterLo =
        (bgPatternShifterLo & 0xFF00) | nextTileLsb;

    bgPatternShifterHi =
        (bgPatternShifterHi & 0xFF00) | nextTileMsb;

    bgAttrShifterLo =
        (bgAttrShifterLo & 0xFF00) |
        ((nextTileAttr & 0x01) ? 0xFF : 0x00);

    bgAttrShifterHi =
        (bgAttrShifterHi & 0xFF00) |
        ((nextTileAttr & 0x02) ? 0xFF : 0x00);
}

uint32_t NES_PPU::getColorFromPalette(uint8_t palette, uint8_t pixel) {
    uint8_t paletteIndex;

    if (pixel == 0)
        paletteIndex = ppuRead(0x3F00);
    else
        paletteIndex = ppuRead(0x3F00 + (palette << 2) + pixel);

    paletteIndex &= 0x3F;

    return nesPalette[paletteIndex];
}