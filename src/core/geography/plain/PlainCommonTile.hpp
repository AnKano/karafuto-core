#pragma once

#include "../../meshes/BaseMesh.hpp"
#include "../tiles/CommonTile.hpp"

namespace KCore {
    class PlainCommonTile {
    public:
        BaseMesh mMesh;
        std::vector<uint8_t> mImage;

        explicit PlainCommonTile(CommonTile *common) : mMesh(*common->getMesh()), mImage(*common->getImage()) {}
    };
}