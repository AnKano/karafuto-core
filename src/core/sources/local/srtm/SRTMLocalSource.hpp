#pragma once

#include "../../BaseSource.hpp"

#include "SRTMFileSourcePiece.hpp"
#include "../../../../bindings.hpp"

namespace KCore {
    class SRTMLocalSource : public BaseSource {
    public:
        uint8_t *getDataForTile(uint8_t zoom, uint16_t x, uint16_t y, uint16_t slicesX, uint16_t slicesY) override;

    protected:
        static void collectTileColumn(const std::vector<std::shared_ptr<BaseSourcePart>> &related,
                                      uint16_t *package,
                                      const float &minimalX, const float &maximalX,
                                      const float &minimalY, const float &maximalY,
                                      const double &offsetX, const double &offsetY,
                                      const uint16_t &slicesX, const uint16_t &slicesY,
                                      bool isWest);

        static void collectTileRow(const std::vector<std::shared_ptr<BaseSourcePart>> &related,
                                   uint16_t *package,
                                   const float &minimalX, const float &maximalX,
                                   const float &minimalY, const float &maximalY,
                                   const double &offsetX, const double &offsetY,
                                   const uint16_t &slicesX, const uint16_t &slicesY,
                                   bool isNorth);

        static void collectTileKernel(const std::vector<std::shared_ptr<BaseSourcePart>> &related,
                                      uint16_t *package,
                                      const float &minimalX, const float &minimalY,
                                      const double &offsetX, const double &offsetY,
                                      const uint16_t &slicesX, const uint16_t &slicesY);

        std::vector<std::shared_ptr<BaseSourcePart>> getRelatedPieces(uint8_t zoom, uint16_t x, uint16_t y) override;

        void createPartFile(const std::string &path) override;
    };

    extern "C" {
    DllExport KCore::SRTMLocalSource *CreateSRTMLocalSource();

    DllExport void SRTMAddPartFile(KCore::SRTMLocalSource *sourcePtr, const char *path);

    DllExport void SRTMAddFileGlob(KCore::SRTMLocalSource *sourcePtr, const char *directory, const char* postfix);
    }
}