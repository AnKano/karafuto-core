#pragma once

#include "../IElevationSrc.hpp"
#include "../../../geography/GeographyConverter.hpp"

#include "SRTMSource.hpp"

namespace KCore {

    class SRTMElevation : public IElevationSrc {
    public:
        SRTMElevation() = default;

        std::vector<float> getTileElevation(uint8_t zoom, uint16_t x, uint16_t y,
                                            uint16_t slicesX, uint16_t slicesY) override {
            const auto kernel = this->getDataForXYZ(zoom, x, y, slicesX, slicesY);
            std::vector<float> results{kernel, kernel + ((slicesX+1) * (slicesY+1))};
            return results;
        }

        float getElevationAtLatLon(float latitude, float longitude) override {
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

        float *getDataForXYZ(uint8_t zoom, uint16_t x, uint16_t y, uint16_t slicesX, uint16_t slicesY) override {
            auto minimalX = GeographyConverter::tileToLon(x, zoom);
            auto maximalY = GeographyConverter::tileToLat(y, zoom);
            auto maximalX = GeographyConverter::tileToLon(x + 1, zoom);
            auto minimalY = GeographyConverter::tileToLat(y + 1, zoom);

            float interVtxGapX = std::abs(minimalX - maximalX) / slicesX;
            float interVtxGapY = std::abs(maximalY - minimalY) / slicesY;

            auto elements = (slicesX + 1) * (slicesY + 1);
            auto *collector = new float[elements];
            std::memset(collector, 0, elements * sizeof(float));

            collectTileKernel(collector, minimalX, minimalY, interVtxGapX, interVtxGapY, slicesX, slicesY);

            return collector;
        }

        void collectTileKernel(float *collectorPtr,
                               const float &minimalX, const float &minimalY, const float &offsetX, const float &offsetY,
                               const uint16_t &slicesX, const uint16_t &slicesY) {
            for (int j = 0; j <= slicesY; j++) {
                for (int i = 0; i <= slicesX; i++) {
                    float pX = minimalX + offsetX * i;
                    float pY = minimalY + offsetY * j;

                    collectorPtr[j * (slicesX+1) + i] = getElevationAtLatLon(pX, pY);
                }
            }
        }
    };

    extern "C" {
    DllExport SRTMElevation* CreateSRTMElevationRepo() {
        return new SRTMElevation();
    }

    DllExport void SRTMElevationRepoAddSource(SRTMElevation* srcPtr, const char* path, SourceType type) {
        switch (type) {
            case SourceFile:
                srcPtr->addSourcePart(new SRTMSource(path, type));
                break;
            case SourceUrl:
                srcPtr->addSourcePart(new SRTMSource(path, type));
                break;
        }
    }
    }
}