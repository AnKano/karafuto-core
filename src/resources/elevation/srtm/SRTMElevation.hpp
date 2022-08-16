#pragma once

#include "SRTMSource.hpp"
#include "../IElevationSource.hpp"
#include "../../../geography/GeographyConverter.hpp"

namespace KCore {
    class SRTMElevation : public IElevationSource {
    public:
        SRTMElevation() = default;

        ~SRTMElevation() = default;

        std::vector<std::vector<float>> getTileElevation
                (const glm::ivec3 &tilecode, const glm::ivec2 &slices) override;

        float getElevationAtLatLon
                (const float &latitude, const float &longitude) override;

        std::vector<std::vector<float>> getDataForXYZ
                (const glm::ivec3 &tilecode, const glm::ivec2 &slices) override;

        void collectTileKernel
                (std::vector<std::vector<float>> &collector,
                 const float &minimalX, const float &minimalY, const float &offsetX, const float &offsetY,
                 const uint16_t &slicesX, const uint16_t &slicesY);
    };

    extern "C" {
    DllExport SRTMElevation *CreateSRTMElevationSource();

    DllExport void AddPieceToSRTMElevationSource
            (SRTMElevation *source_ptr, const char *path, SourceType type);
    }
}