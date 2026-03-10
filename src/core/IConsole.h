#pragma once

#include <cstdint>
#include <string>

class IConsole {
    public:
        virtual ~IConsole() = default;

        virtual void reset() = 0;
        virtual void clock() = 0;

        virtual bool loadCartridge(const std::string& path) = 0;

        virtual const uint32_t* getFrameBuffer() const = 0;
        virtual int getScreenWidth() const = 0;
        virtual int getScreenHeight() const = 0;
};