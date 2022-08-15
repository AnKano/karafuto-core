#include "LayerInterface.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace KCore {
    LayerInterface::LayerInterface() : LayerInterface(0.0f, 0.0f) {}

    LayerInterface::LayerInterface(float latitude, float longitude) : mLayer(latitude, longitude) {}

    LayerInterface::LayerInterface(float latitude, float longitude, const char *url) : mLayer(latitude, longitude) {
        setLayerRasterUrl(url);
    }

    void LayerInterface::updateProjectionMatrix(const glm::mat4 &projectionMatrix) {
        mCameraProjectionMatrix = projectionMatrix;
    }

    void LayerInterface::updateProjectionMatrix(const float *projectionMatrix_ptr, bool transpose) {
        auto cameraProjMatrix = glm::make_mat4x4(projectionMatrix_ptr);
        if (transpose) cameraProjMatrix = glm::transpose(cameraProjMatrix);
        updateProjectionMatrix(cameraProjMatrix);
    }

    void LayerInterface::updateViewMatrix(const glm::mat4 &viewMatrix) {
        mCameraViewMatrix = viewMatrix;
        auto invViewMatrix = glm::inverse(viewMatrix);
        mCameraPosition = invViewMatrix[3];
    }

    void LayerInterface::updateViewMatrix(const float *viewMatrix_ptr, bool transpose) {
        auto cameraViewMatrix = glm::make_mat4x4(viewMatrix_ptr);
        if (transpose) cameraViewMatrix = glm::transpose(cameraViewMatrix);
        updateViewMatrix(cameraViewMatrix);
    }

    void LayerInterface::updateViewMatrixFromParams(const glm::vec3 &positionVector, const glm::vec3 &rotationVector) {
        glm::mat4 viewMatrix(1.0f);
        viewMatrix *= glm::rotate(-rotationVector.x, glm::vec3(1.0f, 0.0f, 0.0f));
        viewMatrix *= glm::rotate(-rotationVector.y, glm::vec3(0.0f, 1.0f, 0.0f));
        viewMatrix *= glm::rotate(-rotationVector.z, glm::vec3(0.0f, 0.0f, 1.0f));
        viewMatrix *= glm::translate(-positionVector);

        updateViewMatrix(viewMatrix);
    }

    void LayerInterface::updateViewMatrixFromParams(const float *positionVector_ptr, const float *rotationVector_ptr) {
        auto position = glm::make_vec3(positionVector_ptr);
        auto rotation = glm::make_vec3(rotationVector_ptr);
        updateViewMatrixFromParams(position, rotation);
    }

    void LayerInterface::update(const float *projectionMatrix_ptr, const float *viewMatrix_ptr,
                                bool transposeProjectionMatrix, bool transposeViewMatrix) {
        updateProjectionMatrix(projectionMatrix_ptr, transposeProjectionMatrix);
        updateViewMatrix(viewMatrix_ptr, transposeViewMatrix);
    }

    void LayerInterface::update(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix) {
        updateProjectionMatrix(projectionMatrix);
        updateViewMatrix(viewMatrix);
    }

    void LayerInterface::calculate() {
        performUpdate();
    }

    void LayerInterface::performUpdate() {
        // !TODO: to parameter
        mCameraProjectionMatrix *= glm::scale(glm::vec3{0.85f, 0.85f, 1.0f});

        mLayer.updateFrustum(mCameraProjectionMatrix, mCameraViewMatrix);
        mLayer.setPosition(mCameraPosition);
        mLayer.update();
    }

    std::vector<LayerEvent> LayerInterface::getCoreEvents() {
        auto res = mLayer.getCoreEventsCopyAndClearQueue();
        return res;
    }

    std::vector<LayerEvent> LayerInterface::getImageEvents() {
        auto res = mLayer.getImageEventsCopyAndClearQueue();
        return res;
    }

    void LayerInterface::setLayerRasterUrl(const char *url) {
        mLayer.setRasterUrl(url);
    }

    Layer *LayerInterface::raw() {
        return &mLayer;
    }

    void LayerInterface::updateProjectionMatrixFromParams(const float fov, const float aspectRatio,
                                                          const float near, const float far) {
        auto projectionMatrix = glm::perspective(fov, aspectRatio, near, far);
        updateProjectionMatrix(projectionMatrix);
    }

    DllExport KCore::LayerInterface *CreateTileLayer(float latitude, float longitude) {
        return new KCore::LayerInterface(latitude, longitude);
    }

    DllExport KCore::LayerInterface *CreateTileLayerWithURL(float latitude, float longitude, const char *url) {
        return new KCore::LayerInterface(latitude, longitude, url);
    }

    DllExport void Update(KCore::LayerInterface *layer_ptr,
                          float *projectionMatrix_ptr, float *viewMatrix_ptr,
                          bool projectionMatrixTranspose, bool viewMatrixTranspose) {
        layer_ptr->update(
                projectionMatrix_ptr, viewMatrix_ptr,
                projectionMatrixTranspose, viewMatrixTranspose
        );
    }

    DllExport void UpdateProjectionMatrix(KCore::LayerInterface *layer_ptr,
                                          float *projectionMatrix_ptr, bool transpose) {
        layer_ptr->updateProjectionMatrix(projectionMatrix_ptr, transpose);
    }

    DllExport void UpdateProjectionMatrixFromParams(KCore::LayerInterface *layer_ptr,
                                                    const float fov, const float aspectRatio,
                                                    const float near, const float far) {
        layer_ptr->updateProjectionMatrixFromParams(fov, aspectRatio, near, far);
    }


    DllExport void UpdateViewMatrix(KCore::LayerInterface *layer_ptr,
                                    float *viewMatrix_ptr, bool transpose) {
        layer_ptr->updateViewMatrix(viewMatrix_ptr, transpose);
    }

    DllExport void UpdateViewMatrixFromParams(KCore::LayerInterface *layer_ptr,
                                              float *positionVector_ptr, float *rotationVector_ptr) {
        layer_ptr->updateViewMatrixFromParams(positionVector_ptr, rotationVector_ptr);
    }

    DllExport void Calculate(KCore::LayerInterface *layer_ptr) {
        layer_ptr->calculate();
    }

    DllExport std::vector<LayerEvent> *GetCoreEventsVector(KCore::LayerInterface *layer_ptr) {
        return new std::vector<LayerEvent>(layer_ptr->getCoreEvents());
    }

    DllExport std::vector<LayerEvent> *GetImageEventsVector(KCore::LayerInterface *layer_ptr) {
        return new std::vector<LayerEvent>(layer_ptr->getImageEvents());
    }

    DllExport LayerEvent *EjectEventsFromVector(std::vector<LayerEvent> *vector_ptr, int &length) {
        length = (int) vector_ptr->size();
        return vector_ptr->data();
    }

    DllExport void ReleaseEventsVector(std::vector<LayerEvent> *vector_ptr) {
        for (const auto &item: *vector_ptr) {
            if (item.type == ImageReady) {
                auto *castedPayload = (ImageResultEvent *) item.payload;
                delete[] castedPayload->data;
                delete castedPayload;
            }
        }

        delete vector_ptr;
    }
}