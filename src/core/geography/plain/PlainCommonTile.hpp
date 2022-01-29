#pragma once

#include "../../../bindings.hpp"

#include "../../meshes/BaseMesh.hpp"
#include "../tiles/CommonTile.hpp"

namespace KCore {
    struct PlainCommonTile {
    public:
        TilePayload mDescription;

        BaseMesh *mMesh;
        std::vector<uint8_t> *mImage;

        explicit PlainCommonTile(CommonTile *common);

        void dispose() const;
    };

    extern "C" {
    DllExport uint8_t *GetImageBytes(std::vector<uint8_t> *image, int &length);
    }
}