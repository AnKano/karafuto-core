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
#include "geography/tiles/CommonTile.hpp"
#include "geography/tiles/MetaTile.hpp"
#include "geography/plain/PlainCommonTile.hpp"
#include "events/MapEvent.hpp"

namespace KCore {
    class MapCore {
    private:
        glm::mat4 mCameraViewMatrix{}, mCameraProjectionMatrix{};
        glm::vec3 mCameraPosition{};

        BaseWorld *mWorld{};

        LimitedSpaceCache<std::shared_ptr<void>> mDataStash;

        std::map<std::string, TileDescription> mCurrentCommonTiles;
        std::map<std::string, TileDescription> mCurrentMetaTiles;

        RenderContext mRenderingContext;
        NetworkContext mNetworkingContext;

        std::mutex mEventsLock;

    public:
        std::vector<KCore::MapEvent> mStoredCommonEvents;
        std::vector<KCore::MapEvent> mStoredMetaEvents;
        std::vector<KCore::MapEvent> mStoredContentEvents;

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

        void pushEventToContentEvent(const MapEvent &event);

#ifdef __EMSCRIPTEN__
        void update(intptr_t camera_projection_matrix_addr,
            intptr_t camera_view_matrix_addr,
            intptr_t camera_position_addr);

        const std::vector<TileDescription>& emscripten_get_tiles();

        const std::vector<TileDescription>& emscripten_get_meta_tiles();
#endif

    private:

        void performUpdate();

        void populateRenderingQueue();
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
    }

}
