#pragma once

#include <array>

#include "../../core/Device.h"
#include "Cartridge.h"

class NES_PPU : public Device {
    public:
        bool nmi = false;
        bool frameComplete = false;

        NES_PPU();

        uint8_t read(uint16_t addr, bool readonly = false) override;
        void write(uint16_t addr, uint8_t data) override;

        void clock();

        inline void connectCartridge(Cartridge* cartridge) { cart = cartridge; }

        uint8_t ppuRead(uint16_t addr);
        void ppuWrite(uint16_t addr, uint8_t data);

        inline const uint32_t* getFrameBuffer() const { return frameBuffer.data(); }

    private:
        Cartridge* cart = nullptr;

        static constexpr int SCREEN_WIDTH = 256;
        static constexpr int SCREEN_HEIGHT = 240;

        int16_t scanline = 0;
        int16_t cycle = 0;

        uint8_t control = 0;    // $2000
        uint8_t mask = 0;       // $2001
        uint8_t status = 0;     // $2002
        uint8_t oamAddr = 0;    // $2003

        uint16_t v = 0;
        uint16_t t = 0;

        uint8_t fineX = 0;
        bool addressLatch = false;

        uint8_t dataBuffer = 0;

        uint8_t nextTileId = 0;
        uint8_t nextTileAttr = 0;
        uint8_t nextTileLsb = 0;
        uint8_t nextTileMsb = 0;

        uint16_t bgPatternShifterLo = 0;
        uint16_t bgPatternShifterHi = 0;

        uint16_t bgAttrShifterLo = 0;
        uint16_t bgAttrShifterHi = 0;

        std::array<uint8_t, 2048> tblName;      // nametables
        std::array<uint8_t, 32> tblPalette;    // palette RAM
        static constexpr uint32_t nesPalette[64] = {
                0xFF545454,0xFF001E74,0xFF081090,0xFF300088,0xFF440064,0xFF5C0030,0xFF540400,0xFF3C1800,
                0xFF202A00,0xFF083A00,0xFF004000,0xFF003C00,0xFF00323C,0xFF000000,0xFF000000,0xFF000000,
                0xFF989698,0xFF084CC4,0xFF3032EC,0xFF5C1EE4,0xFF8814B0,0xFFA01464,0xFF982220,0xFF783C00,
                0xFF545A00,0xFF287200,0xFF087C00,0xFF007628,0xFF006678,0xFF000000,0xFF000000,0xFF000000,
                0xFFECEEEC,0xFF4C9AEC,0xFF787CEC,0xFFB062EC,0xFFE454EC,0xFFEC58B4,0xFFEC6A64,0xFFD48820,
                0xFFA0AA00,0xFF74C400,0xFF4CD020,0xFF38CC6C,0xFF38B4CC,0xFF3C3C3C,0xFF000000,0xFF000000,
                0xFFECEEEC,0xFFA8CCEC,0xFFBCBCEC,0xFFD4B2EC,0xFFECAAE4,0xFFECAED4,0xFFECB4B0,0xFFE4C490,
                0xFFCCD278,0xFFB4DE78,0xFFA8E290,0xFF98E2B4,0xFFA0D6E4,0xFFA0A2A0,0xFF000000,0xFF000000
        };
        std::array<uint32_t, SCREEN_WIDTH * SCREEN_HEIGHT> frameBuffer;

        void incrementScrollX();
        void incrementScrollY();

        void transferAddressX();
        void transferAddressY();

        void updateShifters();
        void loadBackgroundShifters();

        uint32_t getColorFromPalette(uint8_t palette, uint8_t pixel);
};