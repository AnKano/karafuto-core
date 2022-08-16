#pragma once

#include <vector>
#include <memory>

#include "../ISource.hpp"
#include "../../misc/Bindings.hpp"
#include "../primitives/meshes/GridMesh.hpp"

namespace KCore {
    class IElevationSrc {
    protected:
        std::vector<std::unique_ptr<ISource>> mSources;

    public:
        IElevationSrc() = default;

        void addSourcePart(ISource *part) {
            mSources.emplace_back(part);
        }

        virtual std::vector<float> getTileElevation
                (const uint8_t &zoom, const uint16_t &x, const uint16_t &y,
                 const uint16_t &slicesX, const uint16_t &slicesY) = 0;

        virtual float getElevationAtLatLon
                (const float &latitude, const float &longitude) = 0;

        virtual std::vector<float> getDataForXYZ
                (const uint8_t &zoom, const uint16_t &x, const uint16_t &y,
                 const uint16_t &slicesX, const uint16_t &slicesY) = 0;

        GridMesh *createTile
                (const uint8_t &zoom, const uint16_t &x, const uint16_t &y,
                 const uint16_t &slicesX, const uint16_t &slicesY,
                 const bool &flipUVsX, const bool &flipUVsY);
    };

    extern "C" {
    DllExport GridMesh *CreateTileMeshXYZ
            (IElevationSrc *srcPtr,
             uint8_t zoom, uint16_t x, uint16_t y,
             uint16_t slicesX, uint16_t slicesY,
             bool flipUVsX, bool flipUVsY);

    DllExport GridMesh *CreateTileMeshQuadcode
            (IElevationSrc *srcPtr, const char *quadcode,
             uint16_t slicesX, uint16_t slicesY,
             bool flipUVsX, bool flipUVsY);
    }
}