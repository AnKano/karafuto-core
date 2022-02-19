#include "GeoJSONLocalSource.hpp"

#include "../../../geography/GeographyConverter.hpp"

namespace KCore {
    uint8_t *GeoJSONLocalSource::getDataForTile(uint8_t zoom, uint16_t x, uint16_t y,
                                                uint16_t slicesX, uint16_t slicesY) {
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

    std::vector<std::shared_ptr<BaseSourcePart>>
    GeoJSONLocalSource::getRelatedPieces(uint8_t zoom, uint16_t x, uint16_t y) {
        return {};
    }

    void GeoJSONLocalSource::createPartFile(const std::string &path) {
        mPieces.push_back(static_cast<const std::shared_ptr<BaseSourcePart>>(new GeoJSONFileSourcePiece(path)));
    }

    DllExport KCore::GeoJSONLocalSource *CreateGeoJSONLocalSource() {
        return new GeoJSONLocalSource;
    }

    DllExport void GeoJSONAddPartFile(KCore::GeoJSONLocalSource *sourcePtr, const char *path) {
        sourcePtr->addSourcePart(path);
    }
}