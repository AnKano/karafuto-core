#pragma once


#include <vector>
#include <thread>

#include "glm/glm.hpp"

#include "geography/TileDescription.hpp"
#include "worlds/TerrainedWorld.hpp"
#include "meshes/GridMesh.hpp"
#include "contexts/rendering/RenderContext.hpp"
#include "contexts/network/NetworkContext.hpp"
#include "contexts/task/TaskContext.hpp"
#include "cache/LimitedSpaceCache.hpp"
#include "events/MapEvent.hpp"

#include "sources/local/srtm/SRTMLocalSource.hpp"
#include "sources/local/srtm/SRTMFileSourcePiece.hpp"

#include "sources/local/geojson/GeoJSONLocalSource.hpp"
#include "sources/local/geojson/GeoJSONFileSourcePiece.hpp"
#include "sources/local/geojson/primitives/GeoJSONTransObject.hpp"

namespace KCore {
    class MapCore {
    private:
        glm::mat4 mCameraViewMatrix{}, mCameraProjectionMatrix{};
        glm::vec3 mCameraPosition{};

        BaseWorld *mWorldAdapter{nullptr};

    public:
        MapCore();

        void update(const glm::mat4 &cameraProjectionMatrix,
                    const glm::mat4 &cameraViewMatrix,
                    const glm::vec3 &cameraPosition);

        void update(const float *cameraProjectionMatrix_ptr,
                    const float *cameraViewMatrix_ptr,
                    const float *cameraPosition_ptr);

        void update2D(const float &aspectRatio, const float &zoom, const float &cameraPositionX,
                      const float &cameraPositionY);

        void setWorldAdapter(BaseWorld *worldAdapter);

        std::vector<MapEvent> getSyncEvents();

        std::vector<MapEvent> getAsyncEvents();

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
    DllExport KCore::MapCore *CreateMapCore();

    DllExport void SetWorldAdapter(KCore::MapCore *core, KCore::BaseWorld *adapter);

    DllExport void UpdateMapCore(KCore::MapCore *mapCore,
                                 float *cameraProjectionMatrix,
                                 float *cameraViewMatrix,
                                 float *cameraPosition);

    DllExport KCore::MapEvent *GetSyncEvents(KCore::MapCore *mapCore, int &length);

    DllExport void ReleaseSyncEvents(MapEvent *syncArrayPtr);

    DllExport void *GetBufferPtrFromTag(KCore::MapCore *mapCore, const char *tag, int &length);

    DllExport void *GetPoints(std::vector<GeoJSONTransObject> *points, int &length);
    }
}
