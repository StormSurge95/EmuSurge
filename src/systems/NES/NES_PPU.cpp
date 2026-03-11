#include "NES_PPU.h"
#include "../../core/Helpers.h"

NES_PPU::NES_PPU() {
    std::fill(frameBuffer.begin(), frameBuffer.end(), 0);
}

NES_PPU::~NES_PPU() {}

void NES_PPU::clock() {
    if (scanline == -1)
        onPreLine();
    else if (scanline < 240)
        onVisibleLine();
    else if (scanline == 241)
        onVBlankLine();

    cycle++;
    if (cycle >= 341) {
        cycle = 0;
        scanline++;
        if (scanline >= 261) {
            scanline = -1;
            frame++;
            frameComplete = true;
            ntbase = nametableBaseAddr();
            bptbase = bgPatternTblBaseAddr();
        }
    }
}

void NES_PPU::reset() {}

uint8_t NES_PPU::read(uint16_t addr, bool readonly) {
    if (addr >= 0x2000 && addr <= 0x3FFF) {
        addr &= 0x0007;

        switch (addr) {
            case 0x02: // PPUSTATUS
                if (!readonly) w = false;
                return PPUSTATUS;
            case 0x04: // OAMDATA
                return OAMDATA;
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
                break;
            case 0x05: // PPUSCROLL
                PPUSCROLL = data;
                w = !w;
                break;
            case 0x06: // PPUADDR
                if (!w) {
                    vramAddr = (data << 8) | (vramAddr & 0x00FF);
                    w = true;
                } else {
                    vramAddr = (vramAddr & 0xFF00) | data;
                    w = false;
                }
                break;
            case 0x07: // PPU DATA
                ppuWrite(vramAddr, data);
                if (incrementMode())
                    vramAddr += 32;
                else
                    vramAddr += 1;
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

void NES_PPU::plot(uint16_t x, uint16_t y, uint32_t color) {
    frameBuffer[(size_t)y * 256 + (size_t)x] = color;
}

void NES_PPU::onPreLine() {
    if (cycle == 1) {
        isInVblank(false);
    }
}

void NES_PPU::onVisibleLine() {
    if (cycle == 256) renderScanline();
}

void NES_PPU::onVBlankLine() {
    if (cycle == 1) {
        isInVblank(true);
        if (nmiEnabled()) {
            nmiRequested = true;
        }
    }
}

void NES_PPU::renderScanline() {
    uint16_t y = scanline;
    for (uint16_t x = 0; x < 256; x += 8) {
        uint16_t tileX = x / 8;
        tileX %= 32;
        uint16_t tileY = y / 8;
        tileY %= 30;
        uint8_t tileID = getTileID(tileX, tileY);

        for (uint8_t xx = 0; xx < 8; xx++) {
            uint8_t pixel = getPixel(tileID, y % 8, xx);
            uint8_t attr = getAttr(tileX, tileY);
            uint8_t tilePalette = attr * 4 + pixel;
            if (pixel == 0) tilePalette = 0;
            uint8_t color = 0;
            ppuRead(0x3F00 + tilePalette, color);
            uint16_t trueX = x + xx;
            plot(trueX, y, masterPalette[color]);
        }
    }
}

uint8_t NES_PPU::getTileID(uint16_t x, uint16_t y) {
    uint16_t addr = ntbase + (y * 32) + x;
    uint8_t id = 0;
    ppuRead(addr, id);
    return id;
}

uint8_t NES_PPU::getPixel(uint8_t id, uint8_t row, uint8_t x) {
    uint16_t pAddr = bptbase + (uint16_t)id * 16 + row;

    uint8_t p0, p1;
    ppuRead(pAddr, p0);
    ppuRead(pAddr + 8, p1);

    int bit = 7 - x;

    return ((p0 >> bit) & 1) | (((p1 >> bit) & 1) << 1);
}

uint8_t NES_PPU::getAttr(uint16_t x, uint16_t y) {
    uint16_t attrAddr = ntbase + 0x03C0 + (y / 4) * 8 + (x / 4);
    uint8_t attr;
    ppuRead(attrAddr, attr);
    uint8_t shift = ((y % 4) / 2) * 4 +
        ((x % 4) / 2) * 2;

    return (attr >> shift) & 0x03;
}

void NES_PPU::renderPatternTable(uint16_t table, uint16_t screenX, uint16_t screenY) {
    uint16_t base = table * 0x1000;

    for (uint16_t tileY = 0; tileY < 16; tileY++) {
        for (uint16_t tileX = 0; tileX < 16; tileX++) {
            uint16_t tileIndex = tileY * 16 + tileX;
            uint16_t tileAddr = base + tileIndex * 16;

            for (uint8_t row = 0; row < 8; row++) {
                uint8_t plane0, plane1;
                ppuRead(tileAddr + row, plane0);
                ppuRead(tileAddr + row + 8, plane1);

                for (uint8_t col = 0; col < 8; col++) {
                    uint8_t bit = 7 - col;

                    uint8_t pixel = ((plane0 >> bit) & 1) | (((plane1 >> bit) & 1) << 1);
                    uint16_t attrAddr = base + 0x23C0 + (screenY / 4) * 8 + (screenX / 4);
                    uint8_t attrB;
                    ppuRead(attrAddr, attrB);
                    uint8_t shift = ((screenY % 4) / 2) * 4 + ((screenX % 4) / 2) * 2;
                    uint8_t attr = (attrB >> shift) & 0x03;
                    uint8_t pIndex = attr * 4 + pixel;
                    if (pixel == 0) pIndex = 0;
                    uint8_t paletteColor;
                    ppuRead(0x3F00 + pIndex, paletteColor);
                    uint32_t rgb = masterPalette[paletteColor];

                    uint32_t debugPalette[4] = {
                        0x00000000, 0xFF555555,
                        0xFFAAAAAA, 0xFFFFFFFF
                    };

                    plot(screenX + tileX * 8 + col,
                         screenY + tileY * 8 + row,
                         rgb
                    );
                }
            }
        }
    }
}