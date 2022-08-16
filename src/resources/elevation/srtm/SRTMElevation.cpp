#include "SRTMElevation.hpp"

#include <array>

namespace KCore {
    std::vector<std::vector<float>> KCore::SRTMElevation::getTileElevation
            (const glm::ivec3 &tilecode, const glm::ivec2 &slices) {
        return this->getDataForXYZ(tilecode, slices);
    }

    float SRTMElevation::getElevationAtLatLon
            (const float &latitude, const float &longitude) {
        for (const auto &part: mSources) {
            auto *raster = dynamic_cast<SRTMSource *>(part.get());

            double minimalRasterX = raster->mXOrigin, maximalRasterX = raster->mXOpposite;
            double minimalRasterY = raster->mYOpposite, maximalRasterY = raster->mYOrigin;

            if (!(minimalRasterX <= latitude && latitude <= maximalRasterX)) continue;
            if (!(minimalRasterY <= longitude && longitude <= maximalRasterY)) continue;

            double imx = latitude - raster->mXOrigin;
            double imy = raster->mYOrigin - longitude;

            uint32_t row = std::floor(imx / raster->mPixelWidth);
            uint32_t col = std::floor(imy / raster->mPixelHeight);

            uint32_t offset = sizeof(uint16_t) * ((col * 3601) + row);
            auto actual_value = float((raster->mData[offset] << 8) | raster->mData[offset + 1]);

            // avoid extremal height values
            // allow values from 0 m. to  8849 m.
            const auto MAXIMAL_VALUE = 8849.0f;
            actual_value = (actual_value <= MAXIMAL_VALUE) ? actual_value : 0.0f;

            return actual_value;
        }

        return 0.0f;
    }

    std::vector<std::vector<float>> SRTMElevation::getDataForXYZ
            (const glm::ivec3 &tilecode, const glm::ivec2 &slices) {
        auto minimalX = GeographyConverter::tileToLon(tilecode.x, tilecode.z);
        auto maximalY = GeographyConverter::tileToLat(tilecode.y, tilecode.z);
        auto maximalX = GeographyConverter::tileToLon(tilecode.x + 1, tilecode.z);
        auto minimalY = GeographyConverter::tileToLat(tilecode.y + 1, tilecode.z);

        float offsetX = std::abs(minimalX - maximalX) / (float) slices.x;
        float offsetY = std::abs(maximalY - minimalY) / (float) slices.y;

        auto collector = std::vector<std::vector<float>>(slices.x + 1);
        for (int i = 0; i < slices.x + 1; i++)
            collector[i] = std::vector<float>(slices.y + 1);

        collectTileKernel(collector, minimalX, minimalY, offsetX, offsetY, slices.x, slices.y);

        return collector;
    }

    void SRTMElevation::collectTileKernel
            (std::vector<std::vector<float>> &collector,
             const float &minimalX, const float &minimalY,
             const float &offsetX, const float &offsetY,
             const uint16_t &slicesX, const uint16_t &slicesY) {
        for (int j = 0; j <= slicesY; j++) {
            for (int i = 0; i <= slicesX; i++) {
                float pX = minimalX + offsetX * (float) i;
                float pY = minimalY + offsetY * (float) j;

                collector[j][i] = getElevationAtLatLon(pX, pY);
            }
        }
    }

    DllExport SRTMElevation *CreateSRTMElevationSource() {
        return new SRTMElevation();
    }

    DllExport void AddPieceToSRTMElevationSource
            (SRTMElevation *source_ptr,
             const char *path, SourceType type) {
        source_ptr->addSourcePart(new SRTMSource(path, type));
    }
}