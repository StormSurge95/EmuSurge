#include "NES_PPU.h"

NES_PPU::NES_PPU() {}

NES_PPU::~NES_PPU() {}

void NES_PPU::clock() {
    bool rendering = PPUMASK.enableBackground || PPUMASK.enableSprites;

    // VISIBLE PIXEL OUTPUT
    if (scanline >= 0 && scanline < 240 && cycle >= 1 && cycle <= 256) {
        renderPixel();
    }

    // BACKGROUND SHIFT REGISTERS
    if (((cycle >= 2 && cycle <= 257) || (cycle >= 322 && cycle <= 337)))
        shiftBackground();

    // BACKGROUND TILE FETCH PIPELINE
    if (rendering && (scanline <= 239 || scanline == 261) && ((cycle >= 1 && cycle <= 256) || (cycle >= 321 && cycle <= 336))) {
        switch (cycle % 8) {
            case 1:
                loadBackgroundShifters();
                fetchNametableByte();
                break;
            case 3:
                fetchAttributeByte();
                break;
            case 5:
                fetchPatternLow();
                break;
            case 7:
                fetchPatternHigh();
                break;
            case 0:
                incrementX();
                break;
            default:
                break;
        }
    }

    // SCROLL UPDATES
    if (rendering) {
        if (cycle == 256)
            incrementY();
        else if (cycle == 257)
            copyHorizontalBits();
        else if (scanline == 261 && cycle >= 280 && cycle <= 304)
            copyVerticalBits();
    }

    // VBLANK AND PRE-RENDER EVENTS
    if (scanline == 241 && cycle == 1) {
        PPUSTATUS.isInVblank = true;

        if (PPUCTRL.nmi_enabled)
            triggerNMI();
    }
    if (scanline == 261 && cycle == 1) {
        PPUSTATUS.isInVblank = false;
        PPUSTATUS.spriteZeroHit = false;
        PPUSTATUS.spriteOverflow = false;
    }

    // CYCLE AND SCANLINE ADVANCE
    cycle++;

    if (cycle >= 341) {
        cycle = 0;
        scanline++;

        if (scanline >= 262) {
            scanline = 0;
            frameComplete = false;

            oddFrame = !oddFrame;
        }
    }

    // ODD-FRAME CYCLE-SKIP
    if (rendering && oddFrame && scanline == 261 && cycle == 339) {
        cycle = 0;
        scanline = 0;
        frameComplete = true;
        oddFrame = false;
    }
}

void NES_PPU::reset() {}

uint8_t NES_PPU::read(uint16_t addr, bool readonly) {
    if (addr >= 0x2000 && addr <= 0x3FFF) {
        addr &= 0x0007;

        switch (addr) {
            case 0x02: // PPUSTATUS
                if (!readonly) w = false;
                return PPUSTATUS.value();
            case 0x04: // OAMDATA
                if (readonly) return OAMDATA;
                return primaryOAM[OAMADDR];
            case 0x07: // PPUDATA
                return PPUDATA;
        }
    }

    return 0;
}

void NES_PPU::write(uint16_t addr, uint8_t data) {
    if (addr >= 0x2000 && addr <= 0x3FFF) {
        addr &= 0x0007;

        switch (addr) {
            case 0x00: // PPUCTRL
                PPUCTRL = data;
                break;
            case 0x01: // PPUMASK
                PPUMASK = data;
                break;
            case 0x03: // OAMADDR
                OAMADDR = data;
                break;
            case 0x04: // OAMDATA
                OAMDATA = data;
                primaryOAM[OAMADDR] = OAMDATA;
                OAMADDR++;
                break;
            case 0x05: // PPUSCROLL
                PPUSCROLL = data;
                if (!w) {
                    // first write; horizontal scroll (coarse x)
                    fine_x = data & 0x07;

                    t &= ~0x001F;
                    t |= (data >> 3) & 0x1F;

                    w = true;
                } else {
                    // second write; vertical scroll (coarse y)
                    t &= 0x73E0;

                    t |= (data & 0x07) << 12;
                    t |= (data & 0xF8) << 2;

                    w = false;
                }
                break;
            case 0x06: // PPUADDR
                if (!w) {
                    //vramAddr = (data << 8) | (vramAddr & 0x00FF);
                    // first write; high byte of address
                    t &= 0x00FF;
                    t |= (data & 0x3F) << 8;

                    w = true;
                } else {
                    //vramAddr = (vramAddr & 0xFF00) | data;
                    // second write; low byte of address
                    t &= 0x7F00;
                    t |= data;

                    v = t; // transfer address from t to v

                    w = false;
                }
                break;
            case 0x07: // PPU DATA
                //ppuWrite(vramAddr, data);
                ppuWrite(v, data);
                if (PPUCTRL.incrementBy32) {
                    //vramAddr += 32;
                    v += 32;
                } else {
                    //vramAddr += 1;
                    v++;
                }
                break;
        }
    }
}

bool NES_PPU::ppuRead(uint16_t addr, uint8_t& data) {
    addr &= 0x3FFF;

    if (cart->ppuRead(addr, data)) {
        return true;
    } else if (addr >= 0x2000 && addr <= 0x3EFF) {
        addr &= 0x0FFF;
        uint16_t A = addr & 0x03FF;
        uint16_t B = (addr & 0x03FF) + 0x0400;
        if (cart->getMirror() == Cartridge::VERTICAL) {
            if (addr <= 0x03FF) data = nametables[A];
            else if (addr <= 0x07FF) data = nametables[B];
            else if (addr <= 0x0BFF) data = nametables[A];
            else data = nametables[B];
        } else {
            if (addr <= 0x03FF) data = nametables[A];
            else if (addr <= 0x07FF) data = nametables[A];
            else if (addr <= 0x0BFF) data = nametables[B];
            else data = nametables[B];
        }
        return true;
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        addr &= 0x001F;
        if (addr == 0x10) addr = 0x00;
        else if (addr == 0x14) addr = 0x04;
        else if (addr == 0x18) addr = 0x08;
        else if (addr == 0x1C) addr = 0x0C;
        data = paletteRam[addr];
        return true;
    }
    return false;
}

bool NES_PPU::ppuWrite(uint16_t addr, uint8_t data) {
    addr &= 0x3FFF;

    if (cart->ppuWrite(addr, data)) {
        return true;
    } else if (addr >= 0x2000 && addr <= 0x3EFF) {
        addr &= 0x0FFF;

        uint16_t A = addr & 0x03FF;
        uint16_t B = (addr & 0x03FF) + 0x0400;
        if (cart->getMirror() == Cartridge::VERTICAL) {
            if (addr <= 0x03FF) nametables[A] = data;
            else if (addr <= 0x07FF) nametables[B] = data;
            else if (addr <= 0x0BFF) nametables[A] = data;
            else nametables[B] = data;
        } else {
            if (addr <= 0x03FF) nametables[A] = data;
            else if (addr <= 0x07FF) nametables[A] = data;
            else if (addr <= 0x0BFF) nametables[B] = data;
            else nametables[B] = data;
        }
        return true;
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        addr &= 0x1F;
        if (addr == 0x10) addr = 0x00;
        else if (addr == 0x14) addr = 0x04;
        else if (addr == 0x18) addr = 0x08;
        else if (addr == 0x1C) addr = 0x0C;
        paletteRam[addr] = data;
        return true;
    }

    return false;
}

const uint32_t* NES_PPU::getFrameBuffer() const {
    return frameBuffer.data();
}

void NES_PPU::evaluateSprites() {
}

void NES_PPU::renderPixel() {
    uint16_t mux = 0x8000 >> fine_x;

    uint8_t p0 = (pattShiftLo & mux) > 0;
    uint8_t p1 = (pattShiftHi & mux) > 0;

    uint8_t pixel = (p1 << 1) | p0;

    uint8_t a0 = (attrShiftLo & mux) > 0;
    uint8_t a1 = (attrShiftHi & mux) > 0;

    uint8_t attr = (a1 << 1) | a0;

    if (!PPUMASK.enableBackground) {
        pixel = 0;
        attr = 0;
    }

    uint16_t paletteAddr = 0x3F00;

    if (pixel != 0) paletteAddr += (attr << 2) + pixel;

    uint8_t index = 0;
    ppuRead(paletteAddr, index);

    frameBuffer[(size_t)scanline * 256 + ((size_t)cycle - 1)] = masterPalette[index & 0x3F];
}
