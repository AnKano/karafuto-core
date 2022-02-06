#pragma once

#include "../BaseSource.hpp"
#include "SRTMFileSourcePiece.hpp"

namespace KCore {
    class SRTMLocalSource : public BaseSource {
    public:
        uint8_t *getDataForTile(uint8_t zoom, uint16_t x, uint16_t y, uint16_t slicesX, uint16_t slicesY) override {
            auto minimalX = GeographyConverter::tileToLon(x, zoom);
            auto maximalY = GeographyConverter::tileToLat(y, zoom);
            auto maximalX = GeographyConverter::tileToLon(x + 1, zoom);
            auto minimalY = GeographyConverter::tileToLat(y + 1, zoom);

            double offsetX = std::abs(minimalX - maximalX) / (double) slicesX;
            double offsetY = std::abs(maximalY - minimalY) / (double) slicesY;

            auto relatedFiles = getRelatedPieces(zoom, x, y);

            auto *package = new uint16_t[slicesX * slicesY];
            memset(package, 0, slicesX * slicesY * sizeof(uint16_t));

            uint16_t counter = 0;
            uint16_t value = 0;
            for (int j = 0; j < slicesY; j++) {
                for (int i = 0; i < slicesX; i++) {
                    double pX = minimalX + offsetX * i;
                    double pY = minimalY + offsetY * j;
                    pX += offsetX / 2.0f;
                    pY += offsetY / 2.0f;

                    for (const auto &item: relatedFiles) {
                        auto *raster = (SRTMFileSourcePiece *) item.get();

                        double minimalRasterX = raster->mXOrigin;
                        double maximalRasterY = raster->mYOrigin;
                        double maximalRasterX = raster->mXOpposite;
                        double minimalRasterY = raster->mYOpposite;

                        // select raster that contain point
                        if ((minimalRasterX <= pX && pX <= maximalRasterX) &&
                            (minimalRasterY <= pY && pY <= maximalRasterY)) {
                            // hgt-files start from NW corner, but we work in SW-initial
                            double imx = pX - raster->mXOrigin;
                            double imy = raster->mYOrigin - pY;

                            uint32_t row = int(imx / raster->mPixelWidth);
                            uint32_t col = int(imy / raster->mPixelHeight);

                            uint32_t offset = sizeof(uint16_t) * ((col * 3601) + row);

                            value = (raster->mData[offset] << 8) | raster->mData[offset + 1];
                            break;
                        } else
                            continue;
                    }

                    package[counter++] = value;
                }
            }

            return reinterpret_cast<uint8_t *>(package);
        }

    protected:
        std::vector<std::shared_ptr<BaseSourcePiece>> getRelatedPieces(uint8_t zoom, uint16_t x, uint16_t y) override {
            auto minimalX = GeographyConverter::tileToLon(x, zoom);
            auto maximalY = GeographyConverter::tileToLat(y, zoom);
            auto maximalX = GeographyConverter::tileToLon(x + 1, zoom);
            auto minimalY = GeographyConverter::tileToLat(y + 1, zoom);

            std::array<std::array<float, 2>, 4> coords = {
                    {
                            {minimalX, maximalY}, // nw
                            {maximalX, maximalY}, // ne
                            {minimalX, minimalY}, // sw
                            {maximalX, minimalY}, // se
                    }
            };

            std::vector<std::shared_ptr<BaseSourcePiece>> related;

            for (const auto &item: mPieces) {
                auto *raster = (SRTMFileSourcePiece *) item.get();

                double minimalRasterX = raster->mXOrigin;
                double maximalRasterY = raster->mYOrigin;
                double maximalRasterX = raster->mXOpposite;
                double minimalRasterY = raster->mYOpposite;

                for (const auto &coord: coords)
                    if ((minimalRasterX <= coord[0] && coord[0] <= maximalRasterX) &&
                        (minimalRasterY <= coord[1] && coord[1] <= maximalRasterY)) {
                        related.push_back(item);
                        break;
                    }
            }

            return related;
        }
    };
}