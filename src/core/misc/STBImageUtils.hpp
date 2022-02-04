#pragma once

#include <cstdint>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC

#include <stb_image.h>
#include <stdexcept>

namespace KCore::STBImageUtils {
    static std::vector<uint8_t> decodeImageBuffer(const std::vector<uint8_t> &buffer) {
        int width = 0, height = 0, channels = 0;
        unsigned char *data = stbi_load_from_memory(
                (const stbi_uc *) buffer.data(), (int) buffer.size(),
                &width, &height, &channels, 3
        );

        auto resultSize = width * height * channels;

        auto result = std::vector<uint8_t>();
        if (width * height < 0 || channels < 0 || resultSize > result.max_size())
            throw std::runtime_error("can't decode image");

        result.resize(resultSize);
        result.insert(result.begin(), data, data + resultSize);

        // release STB Image data
        delete[] data;

        return result;
    }
}