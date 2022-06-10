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

        virtual std::vector<float> getTileElevation(
                uint8_t zoom, uint16_t x, uint16_t y, uint16_t slicesX, uint16_t slicesY
        ) = 0;

        virtual float getElevationAtLatLon(float latitude, float longitude) = 0;

        void addSourcePart(ISource *part) {
            mSources.emplace_back(part);
        }

        virtual float *getDataForXYZ(uint8_t zoom, uint16_t x, uint16_t y, uint16_t slicesX, uint16_t slicesY) = 0;

        GridMesh *createTile(uint8_t zoom, uint16_t x, uint16_t y, uint16_t slicesX, uint16_t slicesY);
    };

    extern "C" {
    DllExport GridMesh *CreateTileMeshXYZ(
            IElevationSrc *srcPtr, uint8_t zoom, uint16_t x, uint16_t y, uint16_t slicesX, uint16_t slicesY
    );

    DllExport GridMesh *CreateTileMeshQuadcode(
            IElevationSrc *srcPtr, const char *quadcode, uint16_t slicesX, uint16_t slicesY
    );
    }
}