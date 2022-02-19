#pragma once

#include <functional>

namespace KCore {
    class BaseWorld;

    class GenericTile;

    namespace BuiltInResource {
        std::function<void(KCore::BaseWorld *, GenericTile *tile)> ImageCalculate();

        std::function<void(KCore::BaseWorld *, GenericTile *tile)> ImageCalculateMeta();

        std::function<void(KCore::BaseWorld *, GenericTile *tile)> JSONCalculate();

        std::function<void(KCore::BaseWorld *, GenericTile *tile)> TerrainCalculate();
    }
}