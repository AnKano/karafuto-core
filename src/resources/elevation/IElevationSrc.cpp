#include "IElevationSrc.hpp"
#include "../../geography/GeographyConverter.hpp"

namespace KCore {
    GridMesh *IElevationSrc::createTile
            (const uint8_t &zoom, const uint16_t &x, const uint16_t &y,
             const uint16_t &slicesX, const uint16_t &slicesY,
             const bool &flipUVsX, const bool &flipUVsY) {
        auto result = getTileElevation(zoom, x, y, slicesX, slicesY);
        return new GridMesh(1.0f, 1.0f, slicesX, slicesY, flipUVsX, flipUVsY, result.data());
    }

    DllExport GridMesh *CreateTileMeshXYZ
            (IElevationSrc *srcPtr, uint8_t zoom, uint16_t x, uint16_t y,
             uint16_t slicesX, uint16_t slicesY,
             bool flipUVsX, bool flipUVsY) {
        return srcPtr->createTile(zoom, x, y, slicesX, slicesY, flipUVsX, flipUVsY);
    }

    DllExport GridMesh *CreateTileMeshQuadcode
            (IElevationSrc *srcPtr, const char *quadcode,
             uint16_t slicesX, uint16_t slicesY,
             bool flipUVsX, bool flipUVsY) {
        auto tilecode = GeographyConverter::quadcodeToTilecode(quadcode);
        return srcPtr->createTile(tilecode.z, tilecode.x, tilecode.y, slicesX, slicesY, flipUVsX, flipUVsY);
    }
}