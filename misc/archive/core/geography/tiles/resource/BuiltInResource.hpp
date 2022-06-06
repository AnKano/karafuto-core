#pragma once

#include <functional>

namespace KCore {
    class IWorld;

    class GenericTile;

    namespace BuiltInResource {
        std::function<void(KCore::IWorld *, GenericTile *tile)> ImageCalculate();

        std::function<void(KCore::IWorld *, GenericTile *tile)> ImageCalculateMeta();

        std::function<void(KCore::IWorld *, GenericTile *tile)> JSONCalculate();

        std::function<void(KCore::IWorld *, GenericTile *tile)> TerrainCalculate();

        std::function<void(KCore::IWorld *, GenericTile *tile)> JSONWithTerrainAdaptation();
    }
}