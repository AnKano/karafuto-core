#pragma once

#include "../../BaseSource.hpp"

#include "SRTMFileSourcePiece.hpp"
#include "../../../misc/Bindings.hpp"

namespace KCore {
    class SRTMLocalSource : public BaseSource {
    public:
        uint8_t *getDataForTile(uint8_t zoom, uint16_t x, uint16_t y, uint16_t slicesX, uint16_t slicesY) override;

        uint16_t getElevationForLatLonPoint(float latitude, float longitude);

    protected:
        void collectTileKernel(uint16_t *collectorPtr,
                               const float &minimalX, const float &minimalY,
                               const double &offsetX, const double &offsetY,
                               const uint16_t &slicesX, const uint16_t &slicesY);

        uint16_t *getDataForXYZ(const uint8_t &zoom, const uint16_t &x, const uint16_t &y,
                                const uint16_t &slicesX, const uint16_t &slicesY);

        uint16_t parsePoint(double pX, double pY);

        std::vector<std::shared_ptr<BaseSourcePart>> getRelatedPieces(uint8_t zoom, uint16_t x, uint16_t y) override;

        void createPartFile(const std::string &path) override;
    };

    extern "C" {
    DllExport KCore::SRTMLocalSource *CreateSRTMLocalSource();

    DllExport void SRTMAddPartFile(KCore::SRTMLocalSource *sourcePtr, const char *path);

    DllExport void SRTMAddFileGlob(KCore::SRTMLocalSource *sourcePtr, const char *directory, const char *postfix);
    }
}