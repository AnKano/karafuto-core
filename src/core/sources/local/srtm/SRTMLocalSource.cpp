#include "SRTMLocalSource.hpp"
#include "../../../geography/GeographyConverter.hpp"

namespace KCore {
    uint16_t SRTMLocalSource::getElevationForLatLonPoint(float latitude, float longitude, int scale) {
        auto zoom = scale;
        auto x = GeographyConverter::lonToTileX(latitude, scale);
        auto y = GeographyConverter::lat2TileY(longitude, scale);

        auto relatedFiles = getRelatedPieces(zoom, x, y);

        auto pX = latitude;
        auto pY = longitude;

        uint16_t elevation = 0;
        for (const auto &item: relatedFiles) {
            auto *raster = (SRTMFileSourcePiece *) item.get();

            double minimalRasterX = raster->mXOrigin;
            double maximalRasterY = raster->mYOrigin;
            double maximalRasterX = raster->mXOpposite;
            double minimalRasterY = raster->mYOpposite;

            if (minimalRasterX > pX || pX > maximalRasterX ||
                minimalRasterY > pY || pY > maximalRasterY)
                continue;

            // select raster that contain point
            // hgt-files start from NW corner, but we work in SW-initial
            double imx = pX - raster->mXOrigin;
            double imy = raster->mYOrigin - pY;

            uint32_t row = int(imx / raster->mPixelWidth);
            uint32_t col = int(imy / raster->mPixelHeight);

            uint32_t offset = sizeof(uint16_t) * ((col * 3601) + row);

            elevation = (raster->mData[offset] << 8) | raster->mData[offset + 1];
            break;
        }

        return elevation;
    }

    uint8_t *SRTMLocalSource::getDataForTile(uint8_t zoom, uint16_t x, uint16_t y, uint16_t slicesX, uint16_t slicesY) {
        auto minimalX = GeographyConverter::tileToLon(x, zoom);
        auto maximalY = GeographyConverter::tileToLat(y, zoom);
        auto maximalX = GeographyConverter::tileToLon(x + 1, zoom);
        auto minimalY = GeographyConverter::tileToLat(y + 1, zoom);

        double offsetX = std::abs(minimalX - maximalX) / (double) slicesX;
        double offsetY = std::abs(maximalY - minimalY) / (double) slicesY;

        auto relatedFiles = getRelatedPieces(zoom, x, y);
        auto relatedToNorthFiles = getRelatedPieces(zoom, x - 1, y);
        auto relatedToSouthFiles = getRelatedPieces(zoom, x + 1, y);
        auto relatedToWestFiles = getRelatedPieces(zoom, x, y - 1);
        auto relatedToEastFiles = getRelatedPieces(zoom, x, y + 1);

        auto *package = new uint16_t[slicesX * slicesY];

        collectTileKernel(relatedFiles, package, minimalX, minimalY, offsetX, offsetY, slicesX, slicesY);

        collectTileRow(relatedToNorthFiles, package,
                       minimalX, maximalX,
                       minimalY, maximalY,
                       offsetX, offsetY,
                       slicesX, slicesY,
                       true);
        collectTileRow(relatedToNorthFiles, package,
                       minimalX, maximalX,
                       minimalY, maximalY,
                       offsetX, offsetY,
                       slicesX, slicesY,
                       false);

        collectTileColumn(relatedToNorthFiles, package,
                          minimalX, maximalX,
                          minimalY, maximalY,
                          offsetX, offsetY,
                          slicesX, slicesY,
                          true);
        collectTileColumn(relatedToNorthFiles, package,
                          minimalX, maximalX,
                          minimalY, maximalY,
                          offsetX, offsetY,
                          slicesX, slicesY,
                          false);

        return reinterpret_cast<uint8_t *>(package);
    }

    void
    SRTMLocalSource::collectTileColumn(
            const std::vector<std::shared_ptr<BaseSourcePart>> &related, uint16_t *package,
            const float &minimalX, const float &maximalX, const float &minimalY,
            const float &maximalY, const double &offsetX, const double &offsetY,
            const uint16_t &slicesX, const uint16_t &slicesY, bool isWest
    ) {
        double pX;
        int i;
        if (isWest) {
            i = 0;
            pX = minimalX - offsetX;
        } else {
            i = slicesY - 1;
            pX = maximalX;
        }

        for (int j = 0; j < slicesY; j++) {
            double pY = minimalY + offsetY * j;

            for (const auto &item: related) {
                auto *raster = (SRTMFileSourcePiece *) item.get();

                double minimalRasterX = raster->mXOrigin;
                double maximalRasterY = raster->mYOrigin;
                double maximalRasterX = raster->mXOpposite;
                double minimalRasterY = raster->mYOpposite;

                if (minimalRasterX > pX || pX > maximalRasterX ||
                    minimalRasterY > pY || pY > maximalRasterY)
                    continue;

                // select raster that contain point
                // hgt-files start from NW corner, but we work in SW-initial
                double imx = pX - raster->mXOrigin;
                double imy = raster->mYOrigin - pY;

                uint32_t row = int(imx / raster->mPixelWidth);
                uint32_t col = int(imy / raster->mPixelHeight);

                uint32_t offset = sizeof(uint16_t) * ((col * 3601) + row);

                uint16_t result = (raster->mData[offset] << 8) | raster->mData[offset + 1];
                package[j * slicesX + i] = result;
                break;
            }
        }
    }

    void SRTMLocalSource::collectTileRow(
            const std::vector<std::shared_ptr<BaseSourcePart>> &related, uint16_t *package,
            const float &minimalX, const float &maximalX, const float &minimalY,
            const float &maximalY, const double &offsetX, const double &offsetY,
            const uint16_t &slicesX, const uint16_t &slicesY, bool isNorth
    ) {
        double pY;
        int j;
        if (isNorth) {
            j = 0;
            pY = minimalY;
        } else {
            j = slicesX - 1;
            pY = maximalY;
        }

        for (int i = 0; i < slicesX; i++) {
            double pX = minimalX + offsetX * i;

            for (const auto &item: related) {
                auto *raster = (SRTMFileSourcePiece *) item.get();

                double minimalRasterX = raster->mXOrigin;
                double maximalRasterY = raster->mYOrigin;
                double maximalRasterX = raster->mXOpposite;
                double minimalRasterY = raster->mYOpposite;

                if (minimalRasterX > pX || pX > maximalRasterX ||
                    minimalRasterY > pY || pY > maximalRasterY)
                    continue;

                // select raster that contain point
                // hgt-files start from NW corner, but we work in SW-initial
                double imx = pX - raster->mXOrigin;
                double imy = raster->mYOrigin - pY;

                uint32_t row = int(imx / raster->mPixelWidth);
                uint32_t col = int(imy / raster->mPixelHeight);

                uint32_t offset = sizeof(uint16_t) * ((col * 3601) + row);

                uint16_t result = (raster->mData[offset] << 8) | raster->mData[offset + 1];
                package[j * slicesX + i] = result;
                break;
            }
        }
    }

    void
    SRTMLocalSource::collectTileKernel(
            const std::vector<std::shared_ptr<BaseSourcePart>> &related, uint16_t *package,
            const float &minimalX, const float &minimalY, const double &offsetX,
            const double &offsetY, const uint16_t &slicesX, const uint16_t &slicesY
    ) {
        for (int j = 0; j < slicesY; j++) {
            for (int i = 0; i < slicesX; i++) {
                double pX = minimalX + offsetX * i;
                double pY = minimalY + offsetY * j;

                for (const auto &item: related) {
                    auto *raster = (SRTMFileSourcePiece *) item.get();

                    double minimalRasterX = raster->mXOrigin;
                    double maximalRasterY = raster->mYOrigin;
                    double maximalRasterX = raster->mXOpposite;
                    double minimalRasterY = raster->mYOpposite;

                    if (minimalRasterX > pX || pX > maximalRasterX ||
                        minimalRasterY > pY || pY > maximalRasterY)
                        continue;

                    // select raster that contain point
                    // hgt-files start from NW corner, but we work in SW-initial
                    double imx = pX - raster->mXOrigin;
                    double imy = raster->mYOrigin - pY;

                    uint32_t row = int(imx / raster->mPixelWidth);
                    uint32_t col = int(imy / raster->mPixelHeight);

                    uint32_t offset = sizeof(uint16_t) * ((col * 3601) + row);

                    uint16_t result = (raster->mData[offset] << 8) | raster->mData[offset + 1];
                    package[j * slicesX + i] = result;
                    break;
                }
            }
        }
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

    DllExport void SRTMAddFileGlob(KCore::SRTMLocalSource *sourcePtr, const char *directory, const char *postfix) {
        sourcePtr->addSourcePart(directory, postfix);
    }
    }
}