#include "IElevationSrc.hpp"
#include "../../geography/GeographyConverter.hpp"

namespace KCore {
    GridMesh *IElevationSrc::createTile(uint8_t zoom, uint16_t x, uint16_t y, uint16_t slicesX, uint16_t slicesY) {
        auto result = getTileElevation(zoom, x, y, slicesX, slicesY);
        return new GridMesh(1.0f, 1.0f, slicesX, slicesY, result.data());
    }

    DllExport GridMesh *CreateTileMeshXYZ(IElevationSrc *srcPtr, uint8_t zoom, uint16_t x, uint16_t y,
                                          uint16_t slicesX, uint16_t slicesY) {
        return srcPtr->createTile(zoom, x, y, slicesX, slicesY);
    }

    DllExport GridMesh *CreateTileMeshQuadcode(
            IElevationSrc *srcPtr, const char *quadcode, uint16_t slicesX, uint16_t slicesY
    ) {
        auto tilecode = GeographyConverter::quadcodeToTilecode(quadcode);
        return srcPtr->createTile(tilecode.z, tilecode.x, tilecode.y, slicesX, slicesY);
    }
}