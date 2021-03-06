#pragma once

#include <vector>
#include <thread>

#include "glm/glm.hpp"

#include "presenters/IRenderContext.hpp"
#include "events/LayerEvent.hpp"
#include "../geography/TileDescription.hpp"
#include "../resources/primitives/meshes/GridMesh.hpp"
#include "../misc/Bindings.hpp"

#include "Layer.hpp"

namespace KCore {
    enum LayerMode {
        OneToOneLOD,
        OneToSubdivisionLOD
    };

    enum BackendMode {
        NonProcessing = 0,
        Vulkan = 1,
        OpenCL = 2
    };

    class LayerInterface {
    private:
        glm::mat4 mCameraViewMatrix{}, mCameraProjectionMatrix{};
        glm::vec3 mCameraPosition{};

        Layer mLayer;

    public:
        LayerInterface(float latitude, float longitude);

        LayerInterface(float latitude, float longitude, const char* url);

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

        void setBackendMode(BackendMode mode);

        Layer* raw();

    private:
        void setLayerRasterUrl(const char *url);

        void performUpdate();
    };

    extern "C" {
    DllExport KCore::LayerInterface *CreateTileLayerWithURL(float latitude, float longitude, const char* url);
    DllExport KCore::LayerInterface *CreateTileLayerOSM(float latitude, float longitude);
    DllExport void UpdateLayer(KCore::LayerInterface *corePtr,
                               float *cameraProjectionMatrix,
                               float *cameraViewMatrix,
                               float *cameraPosition);
    DllExport std::vector<LayerEvent> *GetEventsVector(KCore::LayerInterface *layerPtr);
    DllExport LayerEvent *EjectEventsFromVector(std::vector<LayerEvent> *vecPtr, int &length);
    DllExport void ReleaseEventsVector(std::vector<LayerEvent> *vecPtr);
    DllExport void SetLayerMode(KCore::LayerInterface *corePtr, LayerMode mode, float param1, float param2);
    DllExport void SetLayerRasterUrl(KCore::LayerInterface *corePtr, const char *url);
    DllExport void SetBackendMode(KCore::LayerInterface *corePtr, BackendMode mode);
    }
}

