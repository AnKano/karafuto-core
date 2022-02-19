#pragma once

#include <functional>

namespace KCore {
    class BaseWorld;

    class GenericTile;

    namespace BuiltInResource {
        std::function<void(KCore::BaseWorld *, GenericTile *tile)> ImageCalculate();

        std::function<void(KCore::BaseWorld *, GenericTile *tile)> JSONCalculate();
    }
}