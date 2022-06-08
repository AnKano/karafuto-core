#pragma once

#include <vector>
#include <thread>

#include "glm/glm.hpp"

#include "presenters/IRenderContext.hpp"
#include "../geography/TileDescription.hpp"
#include "../resources/meshes/GridMesh.hpp"
#include "../events/LayerEvent.hpp"
#include "../misc/Bindings.hpp"

#include "Layer.hpp"

namespace KCore {
    enum LayerMode {
        OneToOneLOD,
        OneToSubdivisionLOD,
//        FixedLevel,
//        FixedLevelWithSubdivision
    };

    class LayerInterface {
    private:
        glm::mat4 mCameraViewMatrix{}, mCameraProjectionMatrix{};
        glm::vec3 mCameraPosition{};

        Layer mWorld;

    public:
        LayerInterface(float latitude, float longitude);

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

        std::vector<LayerEvent> getEvents();

        void setLayerMode(LayerMode mode, float param1, float param2);

        void setLayerRasterUrl(const char *url);

    private:
        void performUpdate();
    };

    extern "C" {
    DllExport KCore::LayerInterface *CreateMapCore(float latitude, float longitude);
    DllExport void UpdateMapCore(KCore::LayerInterface *corePtr,
                                 float *cameraProjectionMatrix,
                                 float *cameraViewMatrix,
                                 float *cameraPosition);
    DllExport std::vector<LayerEvent> *GetEventsVector(KCore::LayerInterface *corePtr);
    DllExport LayerEvent *EjectEventsFromVector(std::vector<LayerEvent> *vecPtr, int &length);
    DllExport void ReleaseEventsVector(std::vector<LayerEvent> *vecPtr);
    DllExport void SetLayerMode(KCore::LayerInterface *corePtr, LayerMode mode, float param1, float param2);
    DllExport void SetLayerRasterUrl(KCore::LayerInterface *corePtr, const char *url);
    }
}

