#include "IElevationSource.hpp"

#include "Elevation.hpp"
#include "../../geography/GeographyConverter.hpp"

namespace KCore {
    GridMesh *IElevationSource::createTile
            (const glm::ivec3 &tilecode, const glm::ivec2 &slices, const glm::bvec2 &flipUVs) {
        auto result = getTileElevation(tilecode, slices);

        return new GridMesh(glm::vec2{1.0f, 1.0f}, slices, flipUVs, Elevation(result));
    }

    DllExport GridMesh *CreateTileMeshXYZ
            (IElevationSource *srcPtr, uint8_t zoom, uint16_t x, uint16_t y,
             uint16_t segmentsX, uint16_t segmentsY,
             bool flipUVsX, bool flipUVsY) {
        auto tilecode = glm::ivec3(zoom, x, y);
        auto segments = glm::ivec2(segmentsX, segmentsY);
        auto flipUVs = glm::bvec2(flipUVsX, flipUVsY);

        return srcPtr->createTile(tilecode, segments, flipUVs);
    }

    DllExport GridMesh *CreateTileMeshQuadcode
            (IElevationSource *srcPtr, const char *quadcode,
             uint16_t segmentsX, uint16_t segmentsY,
             bool flipUVsX, bool flipUVsY) {
        auto tilecode = GeographyConverter::quadcodeToTilecode(quadcode);
        auto segments = glm::ivec2(segmentsX, segmentsY);
        auto flipUVs = glm::bvec2(flipUVsX, flipUVsY);

        return srcPtr->createTile(tilecode, segments, flipUVs);
    }
}