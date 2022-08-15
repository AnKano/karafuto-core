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

        void updateProjectionMatrix(const glm::mat4 &projectionMatrix);
        void updateProjectionMatrix(const float *projectionMatrix_ptr, bool transpose = false);

        void updateProjectionMatrixFromParams(const float fov, const float aspectRatio,
                                              const float near, const float far);

        void updateViewMatrix(const glm::mat4 &viewMatrix);
        void updateViewMatrix(const float *viewMatrix_ptr, bool transpose = false);

        void updateViewMatrixFromParams(const glm::vec3 &positionVector, const glm::vec3 &rotationVector);
        void updateViewMatrixFromParams(const float *positionVector_ptr, const float *rotationVector_ptr);

        void update(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix);
        void update(const float *projectionMatrix_ptr, const float *viewMatrix_ptr,
                    bool transposeProjectionMatrix = false, bool transposeViewMatrix = false);

        void calculate();

        std::vector<LayerEvent> getCoreEvents();

        std::vector<LayerEvent> getImageEvents();

        Layer *raw();

    private:
        void setLayerRasterUrl(const char *url);

        void performUpdate();
    };

    extern "C" {
    DllExport KCore::LayerInterface *CreateTileLayerWithURL(float latitude, float longitude, const char *url);
    DllExport KCore::LayerInterface *CreateTileLayer(float latitude, float longitude);

    DllExport void Update(KCore::LayerInterface *layer_ptr, float *projectionMatrix_ptr, float *viewMatrix_ptr, bool projectionMatrixTranspose = false, bool viewMatrixTranspose = false);

    DllExport void UpdateProjectionMatrix(KCore::LayerInterface *layer_ptr, float *projectionMatrix_ptr, bool transpose = false);
    DllExport void UpdateProjectionMatrixFromParams(KCore::LayerInterface *layer_ptr, float fov, float aspectRatio, float near, float far);

    DllExport void UpdateViewMatrix(KCore::LayerInterface *layer_ptr, float *viewMatrix_ptr, bool transpose = false);
    DllExport void UpdateViewMatrixFromParams(KCore::LayerInterface *layer_ptr, float *positionVector_ptr, float *rotationVector_ptr);

    DllExport void Calculate(KCore::LayerInterface *layer_ptr);

    DllExport std::vector<LayerEvent> *GetCoreEventsVector(KCore::LayerInterface *layer_ptr);
    DllExport std::vector<LayerEvent> *GetImageEventsVector(KCore::LayerInterface *layer_ptr);
    DllExport LayerEvent *EjectEventsFromVector(std::vector<LayerEvent> *vector_ptr, int &length);
    DllExport void ReleaseEventsVector(std::vector<LayerEvent> *vector_ptr);

    DllExport void SetLayerRasterUrl(KCore::LayerInterface *layer_ptr, const char *url);
    }
}