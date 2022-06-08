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

    ImageResult(int width, int height, int channels,
                const std::vector<uint8_t> &imageData): width(width), height(height){
        if (channels == 2)
            format = RGB565;
        else if (channels == 3)
            format = RGB888;
        else if (channels == 4)
            format = RGBA8888;
        size = width * height * channels;
        data = new uint8_t[size];
        std::copy(imageData.begin(), imageData.end(), data);
    }
};
