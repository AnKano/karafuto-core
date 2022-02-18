#pragma once

#include "../TileDescription.hpp"
#include "resource/Resource.hpp"

namespace KCore {
    class BaseWorld;

    class GenericTile {
    private:
        BaseWorld *mWorld;
        TileDescription mDescription;

        // place resource as tag to void ptr
        std::map<std::string, void *> mResources;

        std::vector<std::function<void(BaseWorld *, GenericTile *)>> mImmediateResource;//, mDeferResource;

    public:
        GenericTile(BaseWorld *world, const TileDescription &description);

//        void registerImmediateResource(Resource *resource);

        void registerImmediateResource(const std::function<void(BaseWorld *, GenericTile *)>& callback);

//        void registerDeferResource(Resource *resource) {
//
//        }

        void invokeResources();

        const TileDescription &getTileDescription();
    };
}