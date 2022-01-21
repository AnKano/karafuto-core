#pragma once

#include <utility>

#include "BaseFileSource.hpp"

namespace KCore {
    class TerrainFileSource : public BaseFileSource {
    public:
        TerrainFileSource(std::string filename) : BaseFileSource(std::move(filename)) {}

        std::vector<uint8_t> getRawData() override {
            return mData;
        }

        uint8_t *getTileData(uint8_t zoom, uint16_t x, uint16_t y) override {
            return nullptr;
        }

        std::vector<uint16_t *> getProcessedData(uint8_t zoom, uint16_t x, uint16_t y) {
            return {};
        }
    };
}