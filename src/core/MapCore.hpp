#pragma once

#include <vector>
#include <thread>

#include "glm/glm.hpp"

#include "geography/TileDescription.hpp"
#include "meshes/GridMesh.hpp"

#include "contexts/rendering/IRenderContext.hpp"
#include "contexts/network/INetworkContext.hpp"
#include "contexts/task/TaskContext.hpp"
#include "events/Event.hpp"

#include "World.hpp"

namespace KCore {
    class MapCore {
    private:
        glm::mat4 mCameraViewMatrix{}, mCameraProjectionMatrix{};
        glm::vec3 mCameraPosition{};

        World mWorld;

    public:
        MapCore(float latitude, float longitude);

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

        std::vector<Event> getEvents();

    private:
        void performUpdate();
    };

    extern "C" {
    DllExport KCore::MapCore *CreateMapCore(float latitude, float longitude);

    DllExport void UpdateMapCore(KCore::MapCore *mapCore,
                                 float *cameraProjectionMatrix,
                                 float *cameraViewMatrix,
                                 float *cameraPosition);

    DllExport std::vector<Event> *GetEventsVector(KCore::MapCore *corePtr);

    DllExport KCore::Event *EjectEventsFromVector(std::vector<Event> *vecPtr, int &length);

    DllExport uint8_t* GetBytesFromVector(std::vector<uint8_t> *vecPtr, int &length);

    DllExport void ReleaseArray(uint8_t *arrayPtr);

    DllExport void ReleaseEventsVector(std::vector<Event> *vecPtr);

//    DllExport void *GetPoints(std::vector<GeoJSONTransObject> *points, int &length);
    }
}

