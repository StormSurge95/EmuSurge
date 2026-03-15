#pragma once
#include "../../core/IODevice.h"

#include <memory>

enum NES_BUTTONS {
    BUTTON_A,
    BUTTON_B,
    BUTTON_SELECT,
    BUTTON_START,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT
};

class NES_Controller : public IODevice {
    public:
        bool strobe = false;
        uint8_t cursor = 0x00;
        std::shared_ptr<NES_Controller> other = nullptr;

        NES_Controller(uint8_t player) { this->player = player; }
        ~NES_Controller() = default;

        uint8_t onRead() override;
        void onWrite(uint8_t data) override;

        inline void update(NES_BUTTONS button, bool isPressed) { this->buttons[button] = isPressed; }

    private:
        bool buttons[8] = { false, false, false, false, false, false, false, false };
        uint8_t player = false;
};