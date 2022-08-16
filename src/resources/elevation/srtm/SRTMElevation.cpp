#include "SRTMElevation.hpp"

#include <array>

namespace KCore {
    std::vector<float> KCore::SRTMElevation::getTileElevation
            (const uint8_t &zoom, const uint16_t &x, const uint16_t &y,
             const uint16_t &slicesX, const uint16_t &slicesY) {
        return this->getDataForXYZ(zoom, x, y, slicesX, slicesY);
    }

    float SRTMElevation::getElevationAtLatLon
            (const float &latitude, const float &longitude) {
        for (const auto &part: mSources) {
            auto *raster = (SRTMSource *) part.get();

            double minimalRasterX = raster->mXOrigin;
            double maximalRasterY = raster->mYOrigin;
            double maximalRasterX = raster->mXOpposite;
            double minimalRasterY = raster->mYOpposite;

            if (!(minimalRasterX <= latitude && latitude <= maximalRasterX)) continue;
            if (!(minimalRasterY <= longitude && longitude <= maximalRasterY)) continue;

            double imx = latitude - raster->mXOrigin;
            double imy = raster->mYOrigin - longitude;

            uint32_t row = int(imx / raster->mPixelWidth);
            uint32_t col = int(imy / raster->mPixelHeight);

            uint32_t offset = sizeof(uint16_t) * ((col * 3601) + row);

            uint16_t semiresult = (raster->mData[offset] << 8) | raster->mData[offset + 1];

            return semiresult;
        }

        return 0.0f;
    }

    std::vector<float> SRTMElevation::getDataForXYZ
            (const uint8_t &zoom, const uint16_t &x, const uint16_t &y,
             const uint16_t &slicesX, const uint16_t &slicesY) {
        auto minimalX = GeographyConverter::tileToLon(x, zoom);
        auto maximalY = GeographyConverter::tileToLat(y, zoom);
        auto maximalX = GeographyConverter::tileToLon(x + 1, zoom);
        auto minimalY = GeographyConverter::tileToLat(y + 1, zoom);

        float interVtxGapX = std::abs(minimalX - maximalX) / slicesX;
        float interVtxGapY = std::abs(maximalY - minimalY) / slicesY;

        auto elements = (slicesX + 1) * (slicesY + 1);

        auto collector = std::vector<float>();
        collector.resize(elements);
        std::memset(collector.data(), 0, elements * sizeof(float));

        collectTileKernel(collector.data(), minimalX, minimalY, interVtxGapX, interVtxGapY, slicesX, slicesY);

        return collector;
    }

    void SRTMElevation::collectTileKernel
            (float *collectorPtr, const float &minimalX, const float &minimalY,
             const float &offsetX, const float &offsetY,
             const uint16_t &slicesX, const uint16_t &slicesY) {
        for (int j = 0; j <= slicesY; j++) {
            for (int i = 0; i <= slicesX; i++) {
                float pX = minimalX + offsetX * i;
                float pY = minimalY + offsetY * j;

                collectorPtr[j * (slicesX + 1) + i] = getElevationAtLatLon(pX, pY);
            }
        }
    }

    DllExport SRTMElevation *CreateSRTMElevationSource() {
        return new SRTMElevation();
    }

    DllExport void AddPieceToSRTMElevationSource
            (SRTMElevation *source_ptr, const char *path, SourceType type) {
        source_ptr->addSourcePart(new SRTMSource(path, type));
    }
}