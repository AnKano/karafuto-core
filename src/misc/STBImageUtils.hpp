#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC

#include <stb_image.h>

namespace KCore::STBImageUtils {
    static std::vector<uint8_t> decodeImageBuffer
            (const void *buffer, const std::size_t &length, int &width, int &height, int &channels) {
        unsigned char *data = stbi_load_from_memory
                ((const stbi_uc *) buffer, length, &width, &height, &channels, STBI_default);
        if (width * height < 0 || channels <= 0)
            throw std::runtime_error("can't decode image");

        auto resultSize = width * height * channels;

        std::vector<uint8_t> result{};

        result.reserve(resultSize);
        result.insert(result.begin(), data, data + resultSize);

        // release STB Image data
        delete[] data;

        return result;
    }
}