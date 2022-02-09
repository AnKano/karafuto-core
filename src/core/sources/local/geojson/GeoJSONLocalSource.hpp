#pragma once

#include "../../BaseSource.hpp"

#include "GeoJSONFileSourcePiece.hpp"

namespace KCore {
    class GeoJSONLocalSource : public BaseSource {
    public:
        uint8_t *getDataForTile(uint8_t zoom, uint16_t x, uint16_t y,
                                uint16_t slicesX = 0, uint16_t slicesY = 0) override {
            auto minimalX = GeographyConverter::tileToLon(x, zoom);
            auto maximalX = GeographyConverter::tileToLon(x + 1, zoom);
            auto maximalY = GeographyConverter::tileToLat(y, zoom);
            auto minimalY = GeographyConverter::tileToLat(y + 1, zoom);

            auto objects = new std::vector<GeoJSONObject>{};

            for (const auto &item: mPieces) {
                auto file = (GeoJSONFileSourcePiece *) item.get();
                for (const auto &object: file->getObjects()) {
                    auto &coords = object.mMainShapeCoords;

                    bool relate = false;
                    for (const auto &coord: coords) {
                        if (minimalX > coord[0] || coord[0] > maximalX ||
                            minimalY > coord[1] || coord[1] > maximalY)
                            continue;
                        relate = true;
                        break;
                    }

                    if (relate)
                        objects->push_back(object);
                }
            }

            return reinterpret_cast<uint8_t *>(objects);
        }

    protected:
        std::vector<std::shared_ptr<BaseSourcePiece>> getRelatedPieces(uint8_t zoom, uint16_t x, uint16_t y) override {
            return {};
        }
    };
}