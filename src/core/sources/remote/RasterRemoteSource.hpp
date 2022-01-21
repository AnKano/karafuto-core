#pragma once

#include "BaseRemoteSource.hpp"

namespace KCore {
    class RasterRemoteSource : public BaseRemoteSource {
    public:
        RasterRemoteSource(const std::string &rawUrl) : BaseRemoteSource(rawUrl) {}

        RasterRemoteSource(const std::string &prefix, const std::string &affix) : BaseRemoteSource(prefix, affix) {}

        uint8_t *getTileData(uint8_t zoom, uint16_t x, uint16_t y) override {
            return nullptr;
        }
    };
}