#include "MapCore.hpp"

#include <glm/gtc/type_ptr.hpp>
#include "misc/Utils.hpp"

namespace KCore {
    MapCore::MapCore(float latitude, float longitude) : mWorld(latitude, longitude) {}

    void MapCore::update(const float *cameraProjectionMatrix_ptr,
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

    void MapCore::update(const glm::mat4 &cameraProjectionMatrix,
                         const glm::mat4 &cameraViewMatrix,
                         const glm::vec3 &cameraPosition) {
        mCameraViewMatrix = cameraViewMatrix;
        mCameraProjectionMatrix = cameraProjectionMatrix;
        mCameraPosition = cameraPosition;

        performUpdate();
    }

    void MapCore::update2D(const float &aspectRatio, const float &zoom,
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

    void MapCore::performUpdate() {
        // !TODO: to parameter
        mCameraProjectionMatrix *= glm::scale(glm::vec3{0.85f, 0.85f, 1.0f});

        mWorld.updateFrustum(mCameraProjectionMatrix, mCameraViewMatrix);
        mWorld.setPosition(mCameraPosition);
        mWorld.update();
    }

    std::vector<Event> MapCore::getEvents() {
        return mWorld.getEventsCopyAndClearQueue();
    }

    DllExport KCore::MapCore *CreateMapCore(float latitude, float longitude) {
        return new KCore::MapCore(latitude, longitude);
    }

    DllExport uint8_t *GetVectorMeta(std::vector<uint8_t> *data, int &length) {
        length = data->size();
        return data->data();
    }

    DllExport uint8_t *GetBytesFromVector(std::vector<uint8_t> *vecPtr, int &length) {
        length = vecPtr->size();
        return vecPtr->data();
    }

    DllExport void UpdateMapCore(KCore::MapCore *mapCore,
                                 float *cameraProjectionMatrix,
                                 float *cameraViewMatrix,
                                 float *cameraPosition) {
        mapCore->update(cameraProjectionMatrix,
                        cameraViewMatrix,
                        cameraPosition,
                        false, false);
    }

    DllExport std::vector<Event> *GetEventsVector(KCore::MapCore *corePtr) {
        return new std::vector<Event>(corePtr->getEvents());
    }

    DllExport KCore::Event *EjectEventsFromVector(std::vector<Event> *vecPtr, int &length) {
        length = (int) vecPtr->size();
        return vecPtr->data();
    }

    DllExport void ReleaseEventsVector(std::vector<Event> *vecPtr) {
        for (const auto &item: *vecPtr) {
            if (item.type == EventType::ImageReady) {
                auto *castedPayload = (ImageResult *) item.payload;
                delete[] castedPayload->data;
                delete castedPayload;
            }
        }

        delete vecPtr;
    }

    DllExport void ReleaseArray(uint8_t *arrayPtr) {
        delete[] arrayPtr;
    }

//    DllExport void *GetPoints(std::vector<GeoJSONTransObject> *points, int &length) {
//        if (points == nullptr)
//            length = 0;
//        else
//            length = points->size();
//
//        return points->data();
//    }
}