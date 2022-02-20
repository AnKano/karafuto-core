#include "MapCore.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "worlds/PlainWorld.hpp"
#include "queue/tasks/CallbackTask.hpp"
#include "misc/Utils.hpp"
#include "misc/STBImageUtils.hpp"
#include "meshes/PolylineMesh.hpp"
#include "meshes/PolygonMesh.hpp"

namespace KCore {
    MapCore::MapCore() = default;

    void MapCore::setWorldAdapter(BaseWorld *worldAdapter) {
        mWorldAdapter = worldAdapter;
    }

    void MapCore::update(const float *cameraProjectionMatrix_ptr,
                         const float *cameraViewMatrix_ptr,
                         const float *cameraPosition_ptr) {
        this->mCameraViewMatrix = glm::make_mat4x4(cameraViewMatrix_ptr);
        this->mCameraProjectionMatrix = glm::make_mat4x4(cameraProjectionMatrix_ptr);
        this->mCameraPosition = glm::make_vec3(cameraPosition_ptr);

        performUpdate();
    }

    void MapCore::update(const glm::mat4 &cameraProjectionMatrix,
                         const glm::mat4 &cameraViewMatrix,
                         const glm::vec3 &cameraPosition) {
        this->mCameraViewMatrix = cameraViewMatrix;
        this->mCameraProjectionMatrix = cameraProjectionMatrix;
        this->mCameraPosition = cameraPosition;

        performUpdate();
    }

    void MapCore::update2D(const float &aspectRatio, const float &zoom,
                           const float &cameraPositionX, const float &cameraPositionY) {
        this->mCameraPosition = glm::vec3{-cameraPositionX, zoom, cameraPositionY};
        this->mCameraViewMatrix = glm::lookAt(this->mCameraPosition,
                                              {mCameraPosition.x, 0.0f, mCameraPosition.z},
                                              {0.0f, 0.0f, 1.0f});

        this->mCameraProjectionMatrix = glm::perspective(glm::radians(45.0f),
                                                         aspectRatio,
                                                         100.0f, 2500000.0f);

        performUpdate();
    }

    void MapCore::performUpdate() {
        if (!mWorldAdapter) {
            std::cerr << "MapCore cannot apply matrices: World adapter not found!" << std::endl;
            throw std::runtime_error("Cannot apply matrices: World adapter not found!");
        }

        mWorldAdapter->updateFrustum(this->mCameraProjectionMatrix, this->mCameraViewMatrix);
        mWorldAdapter->setPosition(this->mCameraPosition);
        mWorldAdapter->update();
    }

    std::vector<MapEvent> MapCore::getSyncEvents() {
        return mWorldAdapter->getSyncEvents();
    }

    std::vector<MapEvent> MapCore::getAsyncEvents() {
        return mWorldAdapter->getAsyncEvents();
    }

#ifdef __EMSCRIPTEN__
    void map_core::update(intptr_t camera_projection_matrix_addr,
        intptr_t camera_view_matrix_addr,
        intptr_t camera_position_addr) {
        return update(reinterpret_cast<float*>(camera_projection_matrix_addr),
            reinterpret_cast<float*>(camera_view_matrix_addr),
            reinterpret_cast<float*>(camera_position_addr));
    }


    const std::vector<TileDescription>& map_core::emscripten_get_tiles() {
        mBaseTiles.clear();

        for (const auto& item : get_tiles())
            mBaseTiles.push_back(*item);

        return std::move(mBaseTiles);
    }
    const std::vector<TileDescription>& map_core::emscripten_get_meta_tiles() {
        mMetaTiles.clear();

        for (const auto& item : get_height_tiles())
            mMetaTiles.push_back(*item);

        return std::move(mMetaTiles);
    }
#endif

    DllExport KCore::MapCore *CreateMapCore() {
        return new KCore::MapCore();
    }

    DllExport void SetWorldAdapter(KCore::MapCore *core, KCore::BaseWorld *adapter) {
        core->setWorldAdapter(adapter);
    }

    DllExport void UpdateMapCore(KCore::MapCore *mapCore,
                                 float *cameraProjectionMatrix,
                                 float *cameraViewMatrix,
                                 float *cameraPosition) {
        mapCore->update(cameraProjectionMatrix,
                        cameraViewMatrix,
                        cameraPosition);
    }

    DllExport KCore::MapEvent *GetSyncEvents(KCore::MapCore *corePtr, int &length) {
        auto syncEvents = corePtr->getSyncEvents();

        length = (int) syncEvents.size();

        auto *events = new MapEvent[length];
        std::copy(syncEvents.begin(), syncEvents.end(), events);

        return events;
    }

    DllExport KCore::MapEvent *GetAsyncEvents(KCore::MapCore *corePtr, int &length) {
        auto asyncEvents = corePtr->getAsyncEvents();

        length = (int) asyncEvents.size();

        auto *events = new MapEvent[length];
        std::copy(asyncEvents.begin(), asyncEvents.end(), events);

        return events;
    }

    DllExport void ReleaseEvents(MapEvent *syncArrayPtr) {
        delete[] syncArrayPtr;
    }

    DllExport void *GetPoints(std::vector<GeoJSONTransObject> *points, int &length) {
        if (points == nullptr)
            length = 0;
        else
            length = points->size();

        return points->data();
    }
}