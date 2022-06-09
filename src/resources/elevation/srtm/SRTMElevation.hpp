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

            // collect data for lower row and last column of tile
            const auto south = this->getDataForXYZ(zoom, x, y + 1, slicesX, slicesY);
            const auto east = this->getDataForXYZ(zoom, x + 1, y, slicesX, slicesY);
            // additionally, collect corner data
            const auto southEastCorner = this->getDataForXYZ(zoom, x + 1, y + 1, slicesX, slicesY);

            for (auto y = 0; y < slicesY; y++)
                kernel[y * slicesX + (slicesX - 1)] = east[y * slicesX];

            for (auto x = 0; x < slicesX; x++)
                kernel[x] = south[(slicesY - 1) * slicesX + x];

            kernel[(slicesX - 1)] = southEastCorner[(slicesY - 1) * slicesX + 0];

            std::vector<float> results{kernel, kernel + (slicesX * slicesY * sizeof(float))};
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

            float offsetX = std::abs(minimalX - maximalX) / slicesX;
            float offsetY = std::abs(maximalY - minimalY) / slicesY;

            auto elements = slicesX * slicesY;
            auto *collector = new float[elements];
            std::memset(collector, 0, elements * sizeof(uint16_t));

            collectTileKernel(collector, minimalX, minimalY, offsetX, offsetY, slicesX, slicesY);

            return collector;
        }

        void collectTileKernel(float *collectorPtr,
                               const float &minimalX, const float &minimalY, const float &offsetX, const float &offsetY,
                               const uint16_t &slicesX, const uint16_t &slicesY) {
            for (int j = 0; j < slicesY; j++) {
                for (int i = 0; i < slicesX; i++) {
                    float pX = minimalX + offsetX * i;
                    float pY = minimalY + offsetY * j;

                    auto val = getElevationAtLatLon(pX, pY);
                    collectorPtr[j * slicesX + i] = val;
                }
            }
        }
    };

}