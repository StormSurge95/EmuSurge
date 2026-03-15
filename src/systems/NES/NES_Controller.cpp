#include "../../core/Helpers.h"
#include "NES_Controller.h"

uint8_t NES_Controller::onRead() {
    bool isStrobe = (this->player == 1 ? this->strobe : this->other->strobe);
    if (this->cursor >= 8) return 1;

    if (isStrobe)
        return +(this->buttons[0]);
    else {
        bool isPressed = this->buttons[this->cursor];
        this->cursor++;
        return +isPressed;
    }
}

void NES_Controller::onWrite(uint8_t data) {
    this->strobe = byte.getFlag(data, 0);

    if (this->strobe) {
        this->cursor = 0;
        this->other->cursor = 0;
    }
}

void NES_Controller::update(NES_BUTTONS button, bool isPressed) {
    this->buttons[button] = isPressed;
}