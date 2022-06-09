#include <cstring>
#include "SRTMLocalSource.hpp"
#include "../../../geography/GeographyConverter.hpp"

namespace KCore {
    uint16_t SRTMLocalSource::getElevationForLatLonPoint(float latitude, float longitude) {
        uint16_t elevation = this->parsePoint(latitude, longitude);
        return elevation;
    }

    uint8_t *SRTMLocalSource::getDataForTile(uint8_t zoom, uint16_t x, uint16_t y,
                                             uint16_t slicesX, uint16_t slicesY) {
        // collect tile core data
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

        return reinterpret_cast<uint8_t *>(kernel);
    }

    uint16_t *SRTMLocalSource::getDataForXYZ(const uint8_t &zoom, const uint16_t &x, const uint16_t &y,
                                             const uint16_t &slicesX, const uint16_t &slicesY) {
        auto minimalX = GeographyConverter::tileToLon(x, zoom);
        auto maximalY = GeographyConverter::tileToLat(y, zoom);
        auto maximalX = GeographyConverter::tileToLon(x + 1, zoom);
        auto minimalY = GeographyConverter::tileToLat(y + 1, zoom);

        double offsetX = std::abs(minimalX - maximalX) / (double) slicesX;
        double offsetY = std::abs(maximalY - minimalY) / (double) slicesY;

        auto elements = slicesX * slicesY;
        auto *collector = new uint16_t[elements];
        std::memset(collector, 0, elements * sizeof(uint16_t));

        collectTileKernel(collector, minimalX, minimalY, offsetX, offsetY, slicesX, slicesY);

        return collector;
    }

    void SRTMLocalSource::collectTileKernel(
            uint16_t *collectorPtr,
            const float &minimalX, const float &minimalY, const double &offsetX,
            const double &offsetY, const uint16_t &slicesX, const uint16_t &slicesY
    ) {
        for (int j = 0; j < slicesY; j++) {
            for (int i = 0; i < slicesX; i++) {
                double pX = minimalX + offsetX * i;
                double pY = minimalY + offsetY * j;

                auto val = parsePoint(pX, pY);
                collectorPtr[j * slicesX + i] = val;
            }
        }
    }

    uint16_t SRTMLocalSource::parsePoint(double pX, double pY) {
        uint16_t result = 0;
        for (const auto &part: this->mPieces) {
            auto *raster = (SRTMFileSourcePiece *) part.get();

            double minimalRasterX = raster->mXOrigin;
            double maximalRasterY = raster->mYOrigin;
            double maximalRasterX = raster->mXOpposite;
            double minimalRasterY = raster->mYOpposite;

            if (!(minimalRasterX <= pX && pX <= maximalRasterX)) continue;
            if (!(minimalRasterY <= pY && pY <= maximalRasterY)) continue;

            double imx = pX - raster->mXOrigin;
            double imy = raster->mYOrigin - pY;

            uint32_t row = int(imx / raster->mPixelWidth);
            uint32_t col = int(imy / raster->mPixelHeight);

            uint32_t offset = sizeof(uint16_t) * ((col * 3601) + row);

            return (raster->mData[offset] << 8) | raster->mData[offset + 1];
        }

        return result;
    }


    std::vector<std::shared_ptr<BaseSourcePart>> SRTMLocalSource::getRelatedPieces(
            uint8_t zoom, uint16_t x, uint16_t y
    ) {
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

        std::vector<std::shared_ptr<BaseSourcePart>> related;

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

    void SRTMLocalSource::createPartFile(const std::string &path) {
        auto part = static_cast<const std::shared_ptr<BaseSourcePart>>(new SRTMFileSourcePiece(path));
        mPieces.push_back(part);
    }

    extern "C" {
    DllExport KCore::SRTMLocalSource *CreateSRTMLocalSource() {
        return new SRTMLocalSource;
    }

    DllExport void SRTMAddPartFile(KCore::SRTMLocalSource *sourcePtr, const char *path) {
        sourcePtr->addSourcePart(path);
    }

#ifndef __ANDROID__
    DllExport void SRTMAddFileGlob(KCore::SRTMLocalSource *sourcePtr, const char *directory, const char *postfix) {
        sourcePtr->addSourcePart(directory, postfix);
    }
#endif
    }
}