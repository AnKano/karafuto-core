#pragma once

#include <functional>

#include "presenters/IRenderContext.hpp"
#include "events/LayerEvent.hpp"
#include "../misc/FrustumCulling.hpp"
#include "../geography/TileDescription.hpp"
#include "RemoteSource.hpp"

namespace KCore {
    class Layer {
    private:
        glm::vec2 mOriginLatLon{};
        glm::vec3 mOriginPosition{};

        KCore::FrustumCulling mCullingFilter{};

        std::vector<TileDescription> mTiles{};
        std::map<std::string, TileDescription> mPrevTiles, mCurrTiles{};

        IRenderContext* mRenderContext{nullptr};

        std::map<std::string, bool> mRequested;
        std::unique_ptr<RemoteSource> mRemoteSource{nullptr};

        std::mutex mQueueLock;
        std::vector<LayerEvent> mCoreEventsQueue, mImageEventsQueue;

        std::function<void()> mTileProcessor;

    public:
        Layer();

        Layer(float latitude, float longitude);

        glm::vec2 latLonToWorldPosition(const glm::vec2 &latLon);

        glm::vec2 worldPositionToLatLon(const glm::vec2 &point);

        void updateFrustum(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix);

        void setPosition(const glm::vec3 &position);

        void update();

        void pushToCoreEvents(const LayerEvent &event);

        void pushToImageEvents(const LayerEvent &event);

        std::size_t imageEventsCount();

        std::vector<TileDescription> subdivideSpace(float target = 1.0);

        void calculateTiles();

        bool screenSpaceError(TileDescription &tile, float target, float quality = 3.0f);

        bool checkTileInFrustum(const TileDescription &tile);

        TileDescription createTile(const std::string &quadcode);

        std::vector<LayerEvent> getEventsCopyAndClearQueue();

        // ----------------

        void setRasterUrl(const char* url);

        // ----------------

        void setOneToOneLODMode(float subdivisionTarget = 1.0f);

        void setOneToSubdivisionLODMode(float subdivisionTarget = 1.0f, float additionalSubdivisionTarget = 2.5f);

        // ----------------

#ifdef VULKAN_BACKEND
        void setVulkanMode();
#endif

#ifdef OPENCL_BACKEND
        void setOpenCLMode();
#endif

        void setNonProcessingMode();
    };
}
