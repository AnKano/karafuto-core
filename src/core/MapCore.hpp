#pragma once


#include <vector>
#include <thread>

#include "glm/glm.hpp"

#include "geography/TileDescription.hpp"
#include "worlds/TerrainedWorld.hpp"
#include "meshes/GridMesh.hpp"
#include "contexts/rendering/RenderContext.hpp"
#include "contexts/networking/NetworkContext.hpp"
#include "cache/LimitedSpaceCache.hpp"
#include "events/MapEvent.hpp"

namespace KCore {
    class MapCore {
    private:
        glm::mat4 mCameraViewMatrix{}, mCameraProjectionMatrix{};
        glm::vec3 mCameraPosition{};

        BaseWorld *mWorld{};

        std::map<std::string, TileDescription> mCurrentCommonTiles;
        std::map<std::string, TileDescription> mCurrentMetaTiles;

        RenderContext mRenderingContext{this};
        NetworkContext mNetworkingContext{this, &mRenderingContext};

        std::mutex mEventsLock;

    public:
        LimitedSpaceCache<std::shared_ptr<void>> mDataStash;

        std::vector<KCore::MapEvent> mStoredCommonEvents;
        std::vector<KCore::MapEvent> mStoredMetaEvents;
        std::vector<KCore::MapEvent> mStoredContentEvents;

        std::vector<KCore::MapEvent> mActualContentEvents;
    public:
        MapCore(float latitude, float longitude);

        void update(const glm::mat4 &cameraProjectionMatrix,
                    const glm::mat4 &cameraViewMatrix,
                    const glm::vec3 &cameraPosition);

        void update(const float *cameraProjectionMatrix_ptr,
                    const float *cameraViewMatrix_ptr,
                    const float *cameraPosition_ptr);

        std::vector<MapEvent> getCommonFrameEvents();

        std::vector<MapEvent> getMetaFrameEvents();

        std::vector<MapEvent> getContentFrameEvents();

        void pushEventToContentQueue(const MapEvent &event);

#ifdef __EMSCRIPTEN__
        void update(intptr_t camera_projection_matrix_addr,
            intptr_t camera_view_matrix_addr,
            intptr_t camera_position_addr);

        const std::vector<TileDescription>& emscripten_get_tiles();

        const std::vector<TileDescription>& emscripten_get_meta_tiles();
#endif

    private:
        void performUpdate();
    };

    extern "C" {
    DllExport KCore::MapCore *InstantiateMapCore(float latitude, float longitude);

    DllExport void UpdateMapCore(KCore::MapCore *mapCore,
                                 float *cameraProjectionMatrix,
                                 float *cameraViewMatrix,
                                 float *cameraPosition);

    DllExport KCore::MapEvent *GetCommonFrameEvents(KCore::MapCore *mapCore, int &length);

    DllExport KCore::MapEvent *GetMetaFrameEvents(KCore::MapCore *mapCore, int &length);

    DllExport KCore::MapEvent *GetContentFrameEvents(KCore::MapCore *mapCore, int &length);

    DllExport void *GetBufferPtrFromTag(KCore::MapCore *mapCore, const char *tag, int &length);

    DllExport void ReleaseCopy(const uint8_t* ptr);
    }
}
