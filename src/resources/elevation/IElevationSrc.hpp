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

        GridMesh *createTile
                (const glm::ivec3 &tilecode, const glm::ivec2 &slices, const glm::bvec2 &flipUVs);

        virtual float getElevationAtLatLon
                (const float &latitude, const float &longitude) = 0;

        virtual std::vector<std::vector<float>> getTileElevation
                (const glm::ivec3 &tilecode, const glm::ivec2 &slices) = 0;

        virtual std::vector<std::vector<float>> getDataForXYZ
                (const glm::ivec3 &tilecode, const glm::ivec2 &slices) = 0;
    };

    extern "C" {
    DllExport GridMesh *CreateTileMeshXYZ
            (IElevationSrc *srcPtr,
             uint8_t zoom, uint16_t x, uint16_t y,
             uint16_t segmentsX, uint16_t segmentsY,
             bool flipUVsX, bool flipUVsY);

    DllExport GridMesh *CreateTileMeshQuadcode
            (IElevationSrc *srcPtr, const char *quadcode,
             uint16_t segmentsX, uint16_t segmentsY,
             bool flipUVsX, bool flipUVsY);
    }
}