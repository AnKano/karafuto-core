#include "LayerInterface.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "../misc/Utils.hpp"

namespace KCore {
    LayerInterface::LayerInterface(float latitude, float longitude) : mLayer(latitude, longitude) {}

    LayerInterface::LayerInterface(float latitude, float longitude, const char *url) : mLayer(latitude, longitude) {
        setLayerRasterUrl(url);
    }

    void LayerInterface::update(const float *cameraProjectionMatrix_ptr,
                                const float *cameraViewMatrix_ptr,
                                const float *cameraPosition_ptr,
                                bool transposeProjectionMatrix,
                                bool transposeViewMatrix) {
        auto viewMatrix = glm::make_mat4x4(cameraViewMatrix_ptr);
        if (transposeViewMatrix) viewMatrix = glm::transpose(viewMatrix);

        auto projectionMatrix = glm::make_mat4x4(cameraProjectionMatrix_ptr);
        if (transposeProjectionMatrix) projectionMatrix = glm::transpose(projectionMatrix);

        mCameraViewMatrix = viewMatrix;
        mCameraProjectionMatrix = projectionMatrix;
        mCameraPosition = glm::make_vec3(cameraPosition_ptr);

        performUpdate();
    }

    void LayerInterface::update(const glm::mat4 &cameraProjectionMatrix,
                                const glm::mat4 &cameraViewMatrix,
                                const glm::vec3 &cameraPosition) {
        mCameraViewMatrix = cameraViewMatrix;
        mCameraProjectionMatrix = cameraProjectionMatrix;
        mCameraPosition = cameraPosition;

        performUpdate();
    }

    void LayerInterface::update2D(const float &aspectRatio, const float &zoom,
                                  const float &cameraPositionX, const float &cameraPositionY) {
        mCameraPosition = glm::vec3{-cameraPositionX, zoom, cameraPositionY};
        mCameraViewMatrix = glm::lookAt(mCameraPosition,
                                        {mCameraPosition.x, 0.0f, mCameraPosition.z},
                                        {0.0f, 0.0f, 1.0f});

        mCameraProjectionMatrix = glm::perspective(
                glm::radians(45.0f), aspectRatio, 100.0f, 2500000.0f
        );

        performUpdate();
    }

    void LayerInterface::performUpdate() {
        // !TODO: to parameter
        mCameraProjectionMatrix *= glm::scale(glm::vec3{0.85f, 0.85f, 1.0f});

        mLayer.updateFrustum(mCameraProjectionMatrix, mCameraViewMatrix);
        mLayer.setPosition(mCameraPosition);
        mLayer.update();
    }

    std::vector<LayerEvent> LayerInterface::getEvents() {
        return mLayer.getEventsCopyAndClearQueue();
    }

    void LayerInterface::setLayerMode(LayerMode mode, float param1, float param2) {
        switch (mode) {
            case OneToOneLOD:
                mLayer.setOneToOneLODMode(param1);
                break;
            case OneToSubdivisionLOD:
                mLayer.setOneToSubdivisionLODMode(param1, param2);
                break;
        }
    }

    void LayerInterface::setBackendMode(BackendMode mode) {
        std::cout << "selected mode: " << mode << std::endl;
        switch (mode) {
#ifdef VULKAN_BACKEND
            case Vulkan:
                std::cout << "selected mode: Vulkan" << std::endl;
                mLayer.setVulkanMode();
                break;
#endif
#ifdef OPENCL_BACKEND
            case OpenCL:
                std::cout << "selected mode: OpenCL" << std::endl;
                mLayer.setOpenCLMode();
                break;
#endif
            default:
                std::cout << "selected mode: None" << std::endl;
                mLayer.setNonProcessingMode();
        }
    }

    void LayerInterface::setLayerRasterUrl(const char *url) {
        mLayer.setRasterUrl(url);
    }

    Layer *LayerInterface::raw() {
        return &mLayer;
    }

    DllExport KCore::LayerInterface *CreateTileLayerOSM(float latitude, float longitude) {
        return new KCore::LayerInterface(latitude, longitude);
    }

    DllExport KCore::LayerInterface *CreateTileLayerWithURL(float latitude, float longitude, const char *url) {
        return new KCore::LayerInterface(latitude, longitude, url);
    }

    DllExport void UpdateLayer(KCore::LayerInterface *corePtr,
                               float *cameraProjectionMatrix,
                               float *cameraViewMatrix,
                               float *cameraPosition) {
        corePtr->update(cameraProjectionMatrix,
                        cameraViewMatrix,
                        cameraPosition,
                        false, false);
    }

    DllExport std::vector<LayerEvent> *GetEventsVector(KCore::LayerInterface *layerPtr) {
        return new std::vector<LayerEvent>(layerPtr->getEvents());
    }

    DllExport LayerEvent *EjectEventsFromVector(std::vector<LayerEvent> *vecPtr, int &length) {
        length = (int) vecPtr->size();
        return vecPtr->data();
    }

    DllExport void ReleaseEventsVector(std::vector<LayerEvent> *vecPtr) {
        for (const auto &item: *vecPtr) {
            if (item.type == ImageReady) {
                auto *castedPayload = (ImageResult *) item.payload;
                delete[] castedPayload->data;
                delete castedPayload;
            }
        }

        delete vecPtr;
    }

    DllExport void SetLayerMode(KCore::LayerInterface *corePtr, LayerMode mode, float param1, float param2) {
        corePtr->setLayerMode(mode, param1, param2);
    }

    DllExport void SetBackendMode(KCore::LayerInterface *corePtr, BackendMode mode) {
        corePtr->setBackendMode(mode);
    }
}