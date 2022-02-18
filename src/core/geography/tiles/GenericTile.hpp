#pragma once

#include "../TileDescription.hpp"
//#include "../../worlds/BaseWorld.hpp"
#include "resource/Resource.hpp"

namespace KCore {
    class BaseWorld;

    class GenericTile {
    private:
        BaseWorld *mWorld;
        TileDescription mDescription;

        // place resource as tag to void ptr
        std::map<std::string, void *> mResources;

        std::vector<Resource *> mImmediateResource;//, mDeferResource;

    public:
        GenericTile(BaseWorld *world, const TileDescription &description);

        void registerImmediateResource(Resource *resource);

//        void registerDeferResource(Resource *resource) {
//
//        }

        void invokeResources();

        const TileDescription &getTileDescription();
    };
}