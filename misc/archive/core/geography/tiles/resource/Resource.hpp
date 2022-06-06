#pragma once

#include <functional>

namespace KCore {
    class GenericTile;

    class Resource {
    private:
        std::function<void(GenericTile *)> mPostprocessor = [](GenericTile *tile) {};

    public:
        Resource(std::function<void(GenericTile *)> processor);

        void invoke(GenericTile *tile);
    };
}