#pragma once

#include "glm/glm.hpp"
#include "Layer.hpp"

namespace KCore {
    class LayerInterface {
    private:
        glm::mat4 mCameraViewMatrix{}, mCameraProjectionMatrix{};
        glm::vec3 mCameraPosition{};

        Layer mLayer;

    public:
        LayerInterface();

        LayerInterface(float latitude, float longitude);

        LayerInterface(float latitude, float longitude, const char *url);

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

        std::vector<LayerEvent> getCoreEvents();
        std::vector<LayerEvent> getImageEvents();

        Layer *raw();

    private:
        void setLayerRasterUrl(const char *url);

        void performUpdate();
    };

    extern "C" {
    DllExport KCore::LayerInterface *CreateTileLayerWithURL(float latitude, float longitude, const char *url);
    DllExport KCore::LayerInterface *CreateTileLayerOSM(float latitude, float longitude);
    DllExport void UpdateLayer(KCore::LayerInterface *corePtr,
                               float *cameraProjectionMatrix,
                               float *cameraViewMatrix,
                               float *cameraPosition);
    DllExport std::vector<LayerEvent> *GetEventsVector(KCore::LayerInterface *layerPtr);
    DllExport LayerEvent *EjectEventsFromVector(std::vector<LayerEvent> *vecPtr, int &length);
    DllExport void ReleaseEventsVector(std::vector<LayerEvent> *vecPtr);
    DllExport void SetLayerRasterUrl(KCore::LayerInterface *corePtr, const char *url);
    }
}