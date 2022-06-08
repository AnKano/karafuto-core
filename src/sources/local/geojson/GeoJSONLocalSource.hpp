#pragma once

#include "../../BaseSource.hpp"

#include "GeoJSONFileSourcePiece.hpp"
#include "../../../misc/Bindings.hpp"

namespace KCore {
    class GeoJSONLocalSource : public BaseSource {
    public:
        uint8_t *getDataForTile(uint8_t zoom, uint16_t x, uint16_t y,
                                uint16_t slicesX = 0, uint16_t slicesY = 0) override;

    protected:
        std::vector<std::shared_ptr<BaseSourcePart>> getRelatedPieces(uint8_t zoom, uint16_t x, uint16_t y) override;

        void createPartFile(const std::string &path) override;
    };

    extern "C" {
    DllExport KCore::GeoJSONLocalSource *CreateGeoJSONLocalSource();

    DllExport void GeoJSONAddPartFile(KCore::GeoJSONLocalSource *sourcePtr, const char *path);
    }
}