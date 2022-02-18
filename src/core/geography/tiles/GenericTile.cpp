#include "GenericTile.hpp"

namespace KCore {
    KCore::GenericTile::GenericTile(BaseWorld *world, const TileDescription &description) {
        mWorld = world;
        mDescription = description;
    }

    void GenericTile::registerImmediateResource(const std::string &tag, const GTFunc &callback) {
        mImmediateResource[tag] = callback;
        mCompletedImmediateResource[tag] = false;
    }

    void GenericTile::registerDeferResource(const std::string &tag, const GTFunc &callback,
                                            const std::vector<std::string> &relatedTags) {
        mDeferResource[tag] = callback;
        mDeferResourceRelations[tag] = relatedTags;
    }

    void GenericTile::invokeResources() {
        for (const auto &[resource, resourceFunc]: mImmediateResource)
            resourceFunc(mWorld, this);
    }

    const TileDescription &GenericTile::getTileDescription() {
        return mDescription;
    }

    void GenericTile::commitTag(const std::string &tag) {
        mResourceStatusLock.lock();

        // mark as completed
        mCompletedImmediateResource[tag] = true;

        // create vec of tasks
        auto tasks = std::vector<GTFunc>{};

        // check over all relations
        for (const auto &[resourceTag, resources]: mDeferResourceRelations) {
            auto pos = std::find(resources.begin(), resources.end(), tag);
            if (pos == resources.end()) continue;

            bool isReady = true;
            for (const auto &item: resources)
                if (!mCompletedImmediateResource[item]) {
                    isReady = false;
                    break;
                }

            if (isReady) {
                tasks.push_back(mDeferResource[resourceTag]);
                mDeferResource.erase(resourceTag);
            }
        }

        mResourceStatusLock.unlock();

        for (const auto &item: tasks)
            item(mWorld, this);
    }
}

