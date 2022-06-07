//
// Created by anshu on 6/5/2022.
//

#ifndef KARAFUTO_CORE_WORLD_HPP
#define KARAFUTO_CORE_WORLD_HPP

#include "misc/FrustumCulling.hpp"
#include "geography/TileDescription.hpp"

#include "contexts/task/TaskContext.hpp"
#include "contexts/network/INetworkContext.hpp"
#include "contexts/rendering/IRenderContext.hpp"

#include "sources/RemoteSource.hpp"

#include "events/Event.hpp"

namespace KCore {
    class World {
    private:
        glm::vec2 mOriginLatLon{};
        glm::vec3 mOriginPosition{};

        KCore::FrustumCulling mCullingFilter{};

        std::vector<TileDescription> mTiles{};
        std::map<std::string, TileDescription> mPrevTiles, mCurrTiles{};

        std::unique_ptr<INetworkContext> mNetworkContext{nullptr};
        std::unique_ptr<IRenderContext> mRenderContext{nullptr};

        std::map<std::string, bool> mRequested;
        std::unique_ptr<RemoteSource> mRemoteSource{nullptr};

        std::mutex mQueueLock;
        std::vector<Event> mCoreEventsQueue, mImageEventsQueue;

        std::function<void()> mTileProcessor;

    public:
        World();

        World(float latitude, float longitude);

        glm::vec2 latLonToWorldPosition(const glm::vec2 &latLon);

        glm::vec2 worldPositionToLatLon(const glm::vec2 &point);

        void updateFrustum(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix);

        void setPosition(const glm::vec3 &position);

        virtual void update();

        void pushToCoreEvents(const Event &event);

        void pushToImageEvents(const Event &event);

        std::size_t imageEventsCount();

        std::vector<TileDescription> subdivideSpace(float target = 1.0);

        void calculateTiles();

        bool screenSpaceError(TileDescription &tile, float target, float quality = 3.0f);

        bool checkTileInFrustum(const TileDescription &tile);

        TileDescription createTile(const std::string &quadcode);

        std::vector<Event> getEventsCopyAndClearQueue();

        // ----------------

        void setRasterUrl(const char* url);

        // ----------------

        void setOneToOneLODMode(float subdivisionTarget = 1.0f);

        void setOneToSubdivisionLODMode(float subdivisionTarget = 1.0f, float additionalSubdivisionTarget = 2.5f);
    };
}


#endif //KARAFUTO_CORE_WORLD_HPP
