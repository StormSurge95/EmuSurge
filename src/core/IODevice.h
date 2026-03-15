#pragma once

#include <cstdint>

class IODevice {
    public:
        virtual ~IODevice() = default;

        virtual uint8_t onRead() = 0;
        virtual void onWrite(uint8_t data) = 0;
};