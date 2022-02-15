#pragma once

namespace KCore {
    class BaseSourcePart {
    public:
        BaseSourcePart() = default;

        virtual uint8_t *getTileData(uint8_t zoom, uint16_t x, uint16_t y) = 0;
    };
}