#pragma once

#include <vector>
#include <memory>

#include "../ISource.hpp"

namespace KCore {

    class IElevationSrc {
    protected:
        std::vector<std::unique_ptr<ISource>> mSources;

    public:
        IElevationSrc() = default;

        virtual std::vector<float> getTileElevation(uint8_t zoom, uint16_t x, uint16_t y,
                                                    uint16_t slicesX, uint16_t slicesY) = 0;

        virtual float getElevationAtLatLon(float latitude, float longitude) = 0;

        void addSourcePart(ISource *part) {
            mSources.emplace_back(part);
        }

        virtual float *getDataForXYZ(uint8_t zoom, uint16_t x, uint16_t y, uint16_t slicesX, uint16_t slicesY) = 0;

        float *getDataForTile(uint8_t zoom, uint16_t x, uint16_t y, uint16_t slicesX, uint16_t slicesY) {
            const auto kernel = getDataForXYZ(zoom, x, y, slicesX, slicesY);

            const auto south = getDataForXYZ(zoom, x, y + 1, slicesX, slicesY);
            const auto east = getDataForXYZ(zoom, x + 1, y, slicesX, slicesY);

            const auto southEastCorner = getDataForXYZ(zoom, x + 1, y + 1, slicesX, slicesY);

            for (auto y = 0; y < slicesY; y++)
                kernel[y * slicesX + (slicesX - 1)] = east[y * slicesX];

            for (auto x = 0; x < slicesX; x++)
                kernel[x] = south[(slicesY - 1) * slicesX + x];

            kernel[(slicesX - 1)] = southEastCorner[(slicesY - 1) * slicesX + 0];

            return kernel;
        }

        GridMesh *createTile(uint8_t zoom, uint16_t x, uint16_t y, uint16_t slicesX, uint16_t slicesY) {
            auto result = getDataForTile(zoom, x, y, slicesX, slicesY);

            auto mesh = new GridMesh(1.0f, 1.0f, slicesX - 1, slicesY - 1, result);
//            mesh->applyHeights(result, slicesX, slicesY);

            delete[] result;

            return mesh;
        }

    };

    extern "C" {
    DllExport void CreateTileMeshXYZ(IElevationSrc *srcPtr, uint8_t zoom, uint16_t x, uint16_t y,
                                  uint16_t slicesX, uint16_t slicesY) {
        srcPtr->createTile(zoom, x, y, slicesX, slicesY);
    }

    DllExport void CreateTileMeshQuadcode(IElevationSrc *srcPtr, const char* quadcode, uint16_t slicesX, uint16_t slicesY) {
        auto tilecode = GeographyConverter::quadcodeToTilecode(quadcode);
        srcPtr->createTile(tilecode.z, tilecode.x, tilecode.y, slicesX, slicesY);
    }

    }
}