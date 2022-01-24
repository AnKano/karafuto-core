#pragma once

#include <cstdint>

namespace KCore {
    class BaseSource {
        virtual uint8_t *getTileData(uint8_t zoom, uint16_t x, uint16_t y) = 0;

        virtual bool relateToTileCode(uint8_t zoom, uint16_t x, uint16_t y) = 0;
    };
}