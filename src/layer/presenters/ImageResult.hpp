#pragma once

#include <cstdint>

enum ImageFormat {
    RGB565 = 0,
    RGB888 = 1,
    RGBA8888 = 2
};

struct ImageResult {
    uint32_t width, height;
    ImageFormat format;
    uint64_t size;
    uint8_t *data;
};