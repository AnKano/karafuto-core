#include "GenericTile.hpp"

namespace KCore {
    KCore::GenericTile::GenericTile(BaseWorld *world, const TileDescription &description) {
        mWorld = world;
        mDescription = description;
    }

//    void GenericTile::registerImmediateResource(Resource *resource) {
//        mImmediateResource.push_back(resource);
//    }

    void GenericTile::registerImmediateResource(const std::function<void(BaseWorld *, GenericTile *)> &callback) {
        mImmediateResource.push_back(callback);
    }

    void GenericTile::invokeResources() {
        for (const auto &resource: mImmediateResource)
            resource(mWorld, this);
    }

    const TileDescription &GenericTile::getTileDescription() {
        return mDescription;
    }
}

