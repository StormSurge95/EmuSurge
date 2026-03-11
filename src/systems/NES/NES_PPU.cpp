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

    return cart->read(addr, readonly);
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
                PPUADDR = data;
                w = !w;
                break;
            case 0x07: // PPU DATA
                PPUDATA = data;
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
        uint16_t B = addr & 0x03FF | 0x0400;
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
        data = nametables[addr];
        return true;
    }
    return false
}

bool NES_PPU::ppuWrite(uint16_t addr, uint8_t data) {
    addr &= 0x3FFF;

    if (cart->ppuWrite(addr, data)) {
        return;
    } else if (addr >= 0x2000 && addr <= 0x3EFF) {
        addr &= 0x0FFF;

        uint16_t A = addr & 0x03FF;
        uint16_t B = addr & 0x03FF | 0x0400;
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
    }
}

const uint32_t* NES_PPU::getFrameBuffer() const {
    return frameBuffer.data();
}

void NES_PPU::renderScanline() {
    uint16_t y = scanline;
    for (uint16_t x = 0; x < 256; x += 8) {
        uint16_t tileX = x / 8;
        uint16_t tileY = y / 8;
        uint16_t tileIndex = tileY * 32 + tileX;
        uint16_t tileAddr = nametableBaseAddr() + tileIndex;
        uint16_t tileID = read(tileAddr);

        uint16_t tileInsideY = y % 8;

        uint16_t planeAddr = bgPatternTblBaseAddr() + tileID * 16;
        uint8_t p0 = read(planeAddr + y);
        uint8_t p1 = read(planeAddr + 8 + y);

        for (uint8_t xx = 0; xx < 8; xx++) {
            uint8_t b0 = (p0 >> (7 - xx)) & 0x01;
            uint8_t b1 = (p1 >> (7 - xx)) & 0x01;
            uint8_t color = (b1 << 1) | b0;

            uint32_t palette[4] = { 0xFF000000, 0xFF555555, 0xFFAAAAAA, 0xFFFFFFFF };
            uint16_t trueX = x + xx;
            plot(trueX, y, palette[color]);
        }
    }
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
    if (cycle == 0) renderScanline();
}

void NES_PPU::onVBlankLine() {
    if (cycle == 1) {
        isInVblank(true);
        if (nmiEnabled()) {
            nmiRequested = true;
        }
    }
}