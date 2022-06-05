#pragma once

#include <vector>
#include <thread>

#include "glm/glm.hpp"

#include "geography/TileDescription.hpp"
#include "worlds/TerrainedWorld.hpp"
#include "meshes/GridMesh.hpp"

#include "contexts/rendering/IRenderContext.hpp"
#include "contexts/network/INetworkContext.hpp"
#include "contexts/task/TaskContext.hpp"
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
                    const float *cameraPosition_ptr,
                    bool transposeProjectionMatrix = false,
                    bool transposeViewMatrix = false
        );

        void update2D(const float &aspectRatio, const float &zoom, const float &cameraPositionX,
                      const float &cameraPositionY);

        void setWorldAdapter(BaseWorld *worldAdapter);

        std::vector<MapEvent> getSyncEvents();

        std::vector<MapEvent> getAsyncEvents();

    private:
        void performUpdate();
    };

    extern "C" {
    DllExport KCore::MapCore *CreateMapCore();

    DllExport uint8_t *GetVectorMeta(std::vector<uint8_t> *data, int &length);

    DllExport void SetWorldAdapter(KCore::MapCore *core, KCore::BaseWorld *adapter);

    DllExport void UpdateMapCore(KCore::MapCore *mapCore,
                                 float *cameraProjectionMatrix,
                                 float *cameraViewMatrix,
                                 float *cameraPosition);

    DllExport std::vector<MapEvent> *GetSyncEventsVector(KCore::MapCore *mapCore);

    DllExport KCore::MapEvent *EjectSyncEventsFromVector(std::vector<MapEvent> *vecPtr, int &length);

    DllExport std::vector<MapEvent> *GetAsyncEventsVector(KCore::MapCore *mapCore);

    DllExport KCore::MapEvent *EjectAsyncEventsFromVector(std::vector<MapEvent> *vecPtr, int &length);

    DllExport uint8_t* GetBytesFromVector(std::vector<uint8_t> *vecPtr, int &length);

    DllExport void ReleaseArray(uint8_t *arrayPtr);

    DllExport void ReleaseEventsVector(std::vector<MapEvent> *vecPtr);

    DllExport void *GetPoints(std::vector<GeoJSONTransObject> *points, int &length);
    }
}

