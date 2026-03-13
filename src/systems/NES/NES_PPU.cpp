#include "NES_PPU.h"

NES_PPU::NES_PPU() {
    primaryOAM.resize(256, 0x00);

    secondaryOAM.reserve(32);
    secondaryOAM.clear();
}

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

    // SPRITE EVALUATION
    if (rendering && scanline >= 0 && scanline < 240) {
        if (cycle == 65)
            evaluateSprites();
        else if (cycle == 257)
            fetchSpritePatterns();
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
    if (primaryOAM.size() < 256) return;

    secondaryOAM.clear();

    for (int s = 0; s < 64; s++) {
        int base = s * 4;

        uint8_t top = primaryOAM[base] + 1;

        if (scanline >= top && scanline < (top + spriteHeight())) {
            if (secondaryOAM.size() < 32) {
                secondaryOAM.insert(secondaryOAM.end(), {
                    primaryOAM[base],
                    primaryOAM[base + 1],
                    primaryOAM[base + 2],
                    primaryOAM[base + 3] });
            } else {
                PPUSTATUS.spriteOverflow = true;
                break;
            }
        }
    }
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

    SPRITE_PIXEL spr = getSpritePixel();

    bool bgTrans = pixel == 0;
    bool sprTrans = spr.color == 0;

    if (spr.sprite0 && !bgTrans && !sprTrans)
        PPUSTATUS.spriteZeroHit = true;

    uint8_t finalPixel;
    uint8_t finalPalette;

    if (bgTrans && sprTrans) {
        finalPixel = 0x00;
        finalPalette = 0x00;
    } else if (bgTrans) {
        finalPixel = spr.color;
        finalPalette = 4 + spr.palette;
    } else if (sprTrans) {
        finalPixel = pixel;
        finalPalette = attr;
    } else if (spr.priority == 0) {
        finalPixel = spr.color;
        finalPalette = 4 + spr.palette;
    } else {
        finalPixel = pixel;
        finalPalette = attr;
    }

    uint16_t paletteAddr = 0x3F00;

    if (finalPixel != 0) paletteAddr += (finalPalette << 2) + finalPixel;

    uint8_t index = 0;
    ppuRead(paletteAddr, index);

    frameBuffer[(size_t)scanline * 256 + ((size_t)cycle - 1)] = masterPalette[index & 0x3F];
}

bool NES_PPU::writeDMAByte(uint8_t data) {
    primaryOAM[OAMADDR++] = data;
    return OAMADDR == 0;
}

void NES_PPU::fetchSpritePatterns() {
    uint8_t h = spriteHeight();

    for (int i = 0; i < 8; i++) {
        spriteUnits[i].clear();
    }

    for (uint8_t i = 0; i < secondaryOAM.size(); i += 4) {
        uint8_t unitIndex = i / 4;

        SPRITE s;
        s.yCoord = secondaryOAM[i];
        s.tileIndex = secondaryOAM[i + 1];
        s.attr = SPRITE::ATTR(secondaryOAM[i + 2]);
        s.xCoord = secondaryOAM[i + 3];

        uint8_t row = scanline - (s.yCoord + 1);

        if (s.attr.verticalFlip)
            row = h - 1 - row;

        uint16_t addr = getSpriteAddress(s.tileIndex, (row & (spriteHeight() - 1)));

        uint8_t p0, p1;
        ppuRead(addr, p0);
        ppuRead(addr + 8, p1);

        if (s.attr.horizontalFlip) {
            reverseByte(p0);
            reverseByte(p1);
        }

        spriteUnits[unitIndex].set(s.xCoord, p0, p1, s.attr.value());
    }
}

uint16_t NES_PPU::getSpriteAddress(uint8_t index, uint8_t row) {
    if (PPUCTRL.spritesAre8x16) {
        uint16_t table = (index & 1) * 0x1000;
        uint16_t tile = (index & 0xFE);

        if (row >= 8) {
            tile++;
            row -= 8;
        }

        return table + tile * 16 + row;
    } else {
        uint16_t tileAddr = spritePatternTableBaseAddr() + index * 16;
        return tileAddr + (row & 0x07);
    }
}

NES_PPU::SPRITE_PIXEL NES_PPU::getSpritePixel() {
    for (uint8_t i = 0; i < 8; i++) {
        SPRITE_RENDER_UNIT* u = &spriteUnits[i];

        if (u->xCounter > 0) {
            u->xCounter--;
            continue;
        }

        uint8_t b0 = (u->patternLo >> 7) & 1;
        uint8_t b1 = (u->patternHi >> 7) & 1;

        u->patternLo <<= 1;
        u->patternHi <<= 1;

        uint8_t c = (b1 << 1) | b0;

        if (c == 0) continue;

        uint8_t p = (u->attr >> 2) & 3;
        uint8_t f = (u->attr >> 5) & 1;

        return SPRITE_PIXEL(c, p, f, i == 0);
    }

    return SPRITE_PIXEL();
}

void NES_PPU::reverseByte(uint8_t& b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
}