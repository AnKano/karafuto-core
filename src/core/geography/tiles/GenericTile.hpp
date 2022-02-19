#pragma once

#include <mutex>
#include "../TileDescription.hpp"
#include "resource/Resource.hpp"

namespace KCore {
    class BaseWorld;

    class GenericTile {
    public:
        bool mInvoked = false;
    private:
        typedef std::function<void(BaseWorld *, GenericTile *)> GTFunc;

        BaseWorld *mWorld;
        TileDescription mDescription;

        std::map<std::string, GTFunc> mImmediateResource, mDeferResource;
        std::map<std::string, std::vector<std::string>> mDeferResourceRelations;

        std::mutex mResourceStatusLock;
        std::map<std::string, bool> mCompletedImmediateResource;

        std::vector<std::string> mChildQuadcodes;
        std::vector<std::string> mParentQuadcodes;

    public:
        GenericTile(BaseWorld *world, const TileDescription &description);

        void registerImmediateResource(const std::string &tag, const GTFunc &callback);

        void registerDeferResource(const std::string &tag, const GTFunc &callback,
                                   const std::vector<std::string> &relatedTags);

        void commitTag(const std::string &tag);

        void invokeResources();

        const TileDescription &getTileDescription();

        void setChildQuadcodes(const std::vector<std::string> &quadcodes);

        void setParentQuadcodes(const std::vector<std::string> &quadcodes);
    };
}