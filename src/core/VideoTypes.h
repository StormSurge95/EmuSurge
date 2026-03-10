#pragma once

#include <vector>

struct FrameBuffer {
    int width;
    int height;
    std::vector<uint32_t> pixels;
};