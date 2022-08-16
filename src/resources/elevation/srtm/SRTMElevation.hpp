#pragma once

#include "../IElevationSrc.hpp"
#include "../../../geography/GeographyConverter.hpp"

#include "SRTMSource.hpp"

namespace KCore {
    class SRTMElevation : public IElevationSrc {
    public:
        SRTMElevation() = default;

        std::vector<float> getTileElevation
                (const uint8_t &zoom, const uint16_t &x, const uint16_t &y,
                 const uint16_t &slicesX, const uint16_t &slicesY) override;

        float getElevationAtLatLon
                (const float &latitude, const float &longitude) override;

        std::vector<float> getDataForXYZ
                (const uint8_t &zoom, const uint16_t &x, const uint16_t &y,
                 const uint16_t &slicesX, const uint16_t &slicesY) override;

        void collectTileKernel
                (float *collectorPtr,
                 const float &minimalX, const float &minimalY, const float &offsetX, const float &offsetY,
                 const uint16_t &slicesX, const uint16_t &slicesY);
    };

    extern "C" {
    DllExport SRTMElevation *CreateSRTMElevationSource();

    DllExport void AddPieceToSRTMElevationSource
            (SRTMElevation *source_ptr, const char *path, SourceType type);
    }
}