#include "Mappers/Mapper.h"
#include "NES_PPU.h"

NES_PPU::NES_PPU() : PPUCTRL(0), PPUMASK(0), PPUSTATUS(0) {}

// Performs intra-device read operations on the various
// registers that are visible to other devices for reading.
uint8_t NES_PPU::read(uint16_t addr, bool readonly) {
    if (addr >= 0x2000 && addr <= 0x3FFF) {
        addr &= 0x0007;

        switch (addr) {
            case 0x02: // PPUSTATUS
            {
                uint8_t ret = (this->PPUSTATUS.value() & 0xE0) | (this->ppuBus & 0x1F);
                if (!readonly) {
                    this->PPUSTATUS.isInVblank = false;
                    this->w = false;

                    this->nmiOutput = this->PPUSTATUS.isInVblank && this->PPUCTRL.nmiEnabled;
                }
                this->ppuBus = ret;
                return ret;
            }
            case 0x04: { // OAMDATA
                uint8_t ret = this->primaryOAM[OAMADDR];
                this->ppuBus = ret;
                return ret;
            }
            case 0x07: // PPUDATA
            {
                uint8_t ret = 0x00;

                uint16_t addr = this->v & 0x3FFF;
                uint8_t data = this->ppuRead(addr);

                if (addr >= 0x3F00) {
                    ret = data;
                    this->dataBuffer = this->ppuRead(addr - 0x1000);
                } else {
                    ret = this->dataBuffer;
                    this->dataBuffer = data;
                }

                this->v = (this->v + this->PPUCTRL.incrementMode()) & 0x3FFF;

                this->ppuBus = ret;
                return ret;
            }
        }
    }

    return 0x00;
}

// Performs intra-device write operations on the various
// registers that are visible to other devices for writing.
void NES_PPU::write(uint16_t addr, uint8_t data) {
    this->ppuBus = data;
    if (addr >= 0x2000 && addr <= 0x3FFF) {
        addr &= 0x0007;

        switch (addr) {
            case 0x00: { // PPUCTRL
                bool oldNMI = this->PPUCTRL.nmiEnabled;

                this->PPUCTRL = data;
                this->t = ((this->t & 0xF3FF) | ((uint16_t)(data & 0x03) << 10));

                this->nmiOutput = this->PPUSTATUS.isInVblank && this->PPUCTRL.nmiEnabled;

                if (!this->nmiOutputPrev && this->nmiOutput)
                    this->triggerNMI();

                this->nmiOutputPrev = this->nmiOutput;
                return;
            }
            case 0x01: // PPUMASK
                this->PPUMASK = data;
                return;
            case 0x03: // OAMADDR
                this->OAMADDR = data;
                return;
            case 0x04: // OAMDATA
                this->primaryOAM[OAMADDR] = data;
                this->OAMADDR++;
                return;
            case 0x05: // PPUSCROLL
                this->PPUSCROLL = data;
                if (!this->w) { // first write
                    this->x = (data & 0x07);

                    this->t &= ~0x001F;
                    this->t |= ((uint16_t)(data >> 3) & 0x1F);

                    this->w = true;
                } else { // second write
                    this->t &= 0x0C1F;

                    this->t |= ((uint16_t)(data & 0x07) << 12);
                    this->t |= ((uint16_t)(data & 0xF8) << 2);

                    this->w = false;
                }
                return;
            case 0x06: // PPUADDR
                if (!this->w) { // first write
                    this->t &= 0x00FF;
                    this->t |= ((uint16_t)(data & 0x3F) << 8);

                    this->w = true;
                } else { // second write
                    this->t &= 0x7F00;
                    this->t |= ((uint16_t)data);

                    this->v = this->t & 0x3FFF;

                    this->w = false;
                }
                return;
            case 0x07: // PPUDATA
                this->PPUDATA = data;
                this->ppuWrite(this->v, this->PPUDATA);
                this->v = (this->v + this->PPUCTRL.incrementMode()) & 0x3FFF;
                return;
        }
    }
}

// Performs read operations by reading from VRAM on
// the PPU and/or CHR-ROM/CHR-RAM on the cartridge.
uint8_t NES_PPU::ppuRead(uint16_t addr, bool readonly) {
    if (addr >= 0x0000 && addr <= 0x1FFF)
        return this->cart->mapper->ppuRead(addr, readonly);
    else if (addr >= 0x2000 && addr <= 0x3EFF) {
        addr &= 0x0FFF;
        uint16_t A = addr & 0x03FF;
        uint16_t B = A + 0x0400;
        if (cart->getMirror() == Cartridge::VERTICAL) {
            if (addr <= 0x03FF) return nametables[A];
            if (addr <= 0x07FF) return nametables[B];
            if (addr <= 0x0BFF) return nametables[A];
            return nametables[B];
        } else if (cart->getMirror() == Cartridge::HORIZONTAL) {
            if (addr <= 0x03FF) return nametables[A];
            if (addr <= 0x07FF) return nametables[A];
            if (addr <= 0x0BFF) return nametables[B];
            return nametables[B];
        }
    } else if (addr <= 0x3FFF) {
        addr &= 0x1F;
        if (addr == 0x10) addr = 0x00;
        else if (addr == 0x14) addr = 0x04;
        else if (addr == 0x18) addr = 0x08;
        else if (addr == 0x1C) addr = 0x0C;
        return palettes[addr];
    }

    return 0x00;
}

// Performs write operations by writing to VRAM on
// the PPU and/or CHR-ROM/CHR-RAM on the cartridge.
void NES_PPU::ppuWrite(uint16_t addr, uint8_t data) {
    if (addr >= 0x0000 && addr <= 0x1FFF)
        this->cart->mapper->ppuWrite(addr, data);
    else if (addr >= 0x2000 && addr <= 0x3EFF) {
        addr &= 0x0FFF;
        uint16_t A = addr & 0x03FF;
        uint16_t B = A + 0x0400;
        if (cart->getMirror() == Cartridge::VERTICAL) {
            if (addr <= 0x03FF) {
                nametables[A] = data;
                return;
            }
            if (addr <= 0x07FF) {
                nametables[B] = data;
                return;
            }
            if (addr <= 0x0BFF) {
                nametables[A] = data;
                return;
            }
            nametables[B] = data;
            return;
        } else if (cart->getMirror() == Cartridge::HORIZONTAL) {
            if (addr <= 0x03FF) {
                nametables[A] = data;
                return;
            }
            if (addr <= 0x07FF) {
                nametables[A] = data;
                return;
            }
            if (addr <= 0x0BFF) {
                nametables[B] = data;
                return;
            }
            nametables[B] = data;
            return;
        }
    } else if (addr <= 0x3FFF) {
        addr &= 0x1F;
        if (addr == 0x10) addr = 0x00;
        else if (addr == 0x14) addr = 0x04;
        else if (addr == 0x18) addr = 0x08;
        else if (addr == 0x1C) addr = 0x0C;
        palettes[addr] = data;
    }
}

void NES_PPU::clock() {
    // OUTPUT PROCESSED VISIBLE PIXEL
    if (this->scanline >= 0 && this->scanline <= 239 && this->cycle >= 1 && this->cycle <= 256) {
        this->renderPixel();
    }

    // SHIFT BACKGROUND REGISTERS
    if (this->rendering() && ((this->cycle >= 1 && this->cycle <= 256) || (this->cycle >= 322 && this->cycle <= 337))) {
        this->shiftBackground();
    }

    // FETCH BACKGROUND TILE DATA
    if (this->rendering() && (this->scanline <= 239 || this->scanline == 261) && ((this->cycle >= 1 && this->cycle <= 256) || (this->cycle >= 321 && this->cycle <= 336))) {
        switch (this->cycle % 8) { // 0 - 7
            case 1:
                this->loadBackgroundShifters();
                this->fetchNametableByte();
                break;
            case 3:
                this->fetchAttributeByte();
                break;
            case 5:
                this->fetchPatternLo();
                break;
            case 7:
                this->fetchPatternHi();
                break;
            default:
                break;

        }
        if ((this->cycle % 8) == 0 && ((this->cycle >= 8 && this->cycle <= 256) || this->cycle == 328 || this->cycle == 336))
            this->incrementX();
    }

    // SCROLL UPDATES
    if (this->rendering()) {
        if (cycle == 256) this->incrementY();
        else if (cycle == 257) this->copyHorizontalBits();
        else if (scanline == 261 && cycle >= 280 && cycle <= 304) this->copyVerticalBits();
    }

    // SPRITE EVALUATION
    if (this->rendering() && this->scanline >= 0 && this->scanline < 240) {
        // TODO
    }

    // VBLANK EVENT
    if (this->scanline == 241 && this->cycle == 1) {
        this->PPUSTATUS.isInVblank = true;
    }

    this->nmiOutput = this->PPUSTATUS.isInVblank && this->PPUCTRL.nmiEnabled;
    if (!this->nmiOutputPrev && nmiOutput)
        this->triggerNMI();

    this->nmiOutputPrev = nmiOutput;

    // PRE-RENDER EVENT
    if (this->scanline == 260 && this->cycle == 331) {
        this->PPUSTATUS.isInVblank = false;
        this->PPUSTATUS.spriteOverflow = false;
        this->PPUSTATUS.spriteZeroHit = false;
    }

    // CYCLE & SCANLINE ADVANCE
    this->cycle++;

    if (this->cycle >= 341) {
        this->cycle = 0;
        this->scanline++;

        if (this->scanline >= 262) {
            this->scanline = 0;
            this->frameComplete = true;
            this->oddFrame = !this->oddFrame;
        }
    }

    // ODD-FRAME CYCLE-SKIP
    if (this->rendering() && this->oddFrame && this->scanline == 261 && this->cycle == 339) {
        this->cycle = 0;
        this->scanline = 0;
        this->frameComplete = true;
        this->oddFrame = false;
    }
}

void NES_PPU::incrementX() {
    if ((this->v & 0x001F) == 31) {
        this->v &= ~0x001F;
        this->v ^= 0x0400;
    } else {
        this->v++;
    }
}

void NES_PPU::incrementY() {
    if ((this->v & 0x7000) != 0x7000) {
        this->v += 0x1000;
    } else {
        this->v &= ~0x7000;
        uint8_t y = this->coarseY();
        if (y == 29) {
            y = 0;
            this->v ^= 0x0800;
        } else if (y == 31) {
            y = 0;
        } else {
            y++;
        }
        this->v = (this->v & ~0x03E0) | ((uint16_t)y << 5);
    }
}

void NES_PPU::shiftBackground() {
    if (this->PPUMASK.enableBackground) {
        patternShiftLo <<= 1;
        patternShiftHi <<= 1;

        attributeShiftLo <<= 1;
        attributeShiftHi <<= 1;
    }
}

void NES_PPU::loadBackgroundShifters() {
    this->patternShiftLo = (this->patternShiftLo & 0xFF00) | this->nextPatternByteLo;
    this->patternShiftHi = (this->patternShiftHi & 0xFF00) | this->nextPatternByteHi;

    this->attributeShiftLo = (this->attributeShiftLo & 0xFF00) | ((this->nextAttributeByte & 0b01) ? 0xFF : 0x00);
    this->attributeShiftHi = (this->attributeShiftHi & 0xFF00) | ((this->nextAttributeByte & 0b10) ? 0xFF : 0x00);
}

void NES_PPU::fetchNametableByte() {
    uint16_t addr = (0x2000 | (this->v & 0x0FFF));
    this->nextNametableByte = this->ppuRead(addr, false);
}

void NES_PPU::fetchAttributeByte() {
    uint16_t addr = 0x23C0 |
        (this->v & 0x0C00) |
        ((this->v >> 4) & 0x38) |
        ((this->v >> 2) & 0x07);
    this->nextAttributeByte = this->ppuRead(addr, false);

    if (this->coarseY() & 0x02) this->nextAttributeByte >>= 4;
    if (this->coarseX() & 0x02) this->nextAttributeByte >>= 2;

    this->nextAttributeByte &= 0x03;
}

void NES_PPU::fetchPatternLo() {
    uint16_t addr = this->PPUCTRL.backgroundPatternSelect() |
        (((uint16_t)this->nextNametableByte) << 4) |
        ((uint16_t)this->fineY());
    this->nextPatternByteLo = this->ppuRead(addr, false);
}

void NES_PPU::fetchPatternHi() {
    uint16_t addr = this->PPUCTRL.backgroundPatternSelect() |
        (((uint16_t)this->nextNametableByte) << 4) |
        ((uint16_t)this->fineY());
    this->nextPatternByteHi = this->ppuRead(addr + 8, false);
}

void NES_PPU::renderPixel() {
    uint16_t mux = 0x8000 >> this->x;

    uint8_t p0 = (this->patternShiftLo & mux) > 0;
    uint8_t p1 = (this->patternShiftHi & mux) > 0;

    uint8_t pixel = (p1 << 1) | p0;

    uint8_t a0 = (this->attributeShiftLo & mux) > 0;
    uint8_t a1 = (this->attributeShiftHi & mux) > 0;

    uint8_t attr = (a1 << 1) | a0;

    if (!this->PPUMASK.enableBackground) {
        pixel = 0;
        attr = 0;
    }

    if ((this->cycle - 1) < 8) {
        if (!this->PPUMASK.enableBackgroundLeft) {
            pixel = 0;
            attr = 0;
        }
    }

    uint16_t paletteAddr = 0x3F00;

    if (pixel != 0) paletteAddr += ((attr << 2) + pixel);

    uint8_t index = this->ppuRead(paletteAddr, false);

    this->frameBuffer[(size_t)this->scanline * 256 + ((size_t)this->cycle - 1)] = this->masterPalette[index & 0x3F];
}