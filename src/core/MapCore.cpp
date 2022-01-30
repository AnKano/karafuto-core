#include <glm/gtc/type_ptr.hpp>

#include "MapCore.hpp"

#include "sources/remote/RasterRemoteSource.hpp"
#include "worlds/PlainWorld.hpp"
#include "queue/tasks/CallbackTask.hpp"
#include "misc/Utils.hpp"

namespace KCore {
    MapCore::MapCore(float latitude, float longitude) {
        glm::vec2 origin_lat_lon{latitude, longitude};
        glm::vec2 origin_point{GeographyConverter::latLonToPoint(origin_lat_lon)};

        KCore::WorldConfig config{};
        config.GenerateMeta = true;

        mWorld = new TerrainedWorld(origin_lat_lon, origin_point, config);
        mWorld->updateFrustum(this->mCameraProjectionMatrix, this->mCameraViewMatrix);
        mWorld->setPosition(this->mCameraPosition);

        mDataStash.setMaximalCount(5000);
        mDataStash.setCheckInterval(10);
        mDataStash.setStayAliveInterval(20);
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

    void MapCore::performUpdate() {
        if (!mWorld) return;

        mWorld->updateFrustum(this->mCameraProjectionMatrix, this->mCameraViewMatrix);
        mWorld->setPosition(this->mCameraPosition);
        mWorld->update();
    }

    std::vector<MapEvent> MapCore::getCommonFrameEvents() {
        if (!mWorld) throw std::runtime_error("world not initialized");

        auto previousFrameTilesCopy = mCurrentCommonTiles;

        auto tiles = mWorld->getTiles();
        std::map<std::string, TileDescription> currentFrameTiles;
        for (const auto &item: tiles)
            currentFrameTiles[item.getQuadcode()] = item;

        mCurrentCommonTiles = currentFrameTiles;

        auto diff = mapKeysDifference<std::string>(previousFrameTilesCopy, currentFrameTiles);
        auto inter = mapKeysIntersection<std::string>(previousFrameTilesCopy, currentFrameTiles);

        auto events = std::vector<MapEvent>();
        for (const auto &item: diff) {
            if (previousFrameTilesCopy.find(item) != std::end(previousFrameTilesCopy)) {
                MapEvent event{};
                event.Type = NotInFrustum;
                strcpy_s(event.Quadcode, previousFrameTilesCopy[item].getQuadcode().c_str());
                event.OptionalPayload = nullptr;
                events.push_back(event);
                continue;
            }

            if (currentFrameTiles.find(item) != std::end(currentFrameTiles)) {
                MapEvent event{};
                event.Type = InFrustum;
                strcpy_s(event.Quadcode, mCurrentCommonTiles[item].getQuadcode().c_str());
                event.OptionalPayload = (void *) &mCurrentCommonTiles[item].mPayload;
                events.push_back(event);
                continue;
            }
        }

        mStoredCommonEvents = events;

        return mStoredCommonEvents;
    }

    std::vector<MapEvent> MapCore::getMetaFrameEvents() {
        if (!mWorld) throw std::runtime_error("world not initialized");

        auto previousFrameTilesCopy = mCurrentMetaTiles;

        auto tiles = ((TerrainedWorld *) mWorld)->getMetaTiles();
        std::map<std::string, TileDescription> currentFrameTiles;
        for (const auto &item: tiles)
            currentFrameTiles[item.getQuadcode()] = item;

        mCurrentMetaTiles = currentFrameTiles;

        auto diff = mapKeysDifference<std::string>(previousFrameTilesCopy, currentFrameTiles);
        auto inter = mapKeysIntersection<std::string>(previousFrameTilesCopy, currentFrameTiles);

        auto events = std::vector<MapEvent>();
        for (const auto &item: diff) {
            if (previousFrameTilesCopy.find(item) != std::end(previousFrameTilesCopy)) {
                MapEvent event{};
                event.Type = NotInFrustum;
                strcpy_s(event.Quadcode, previousFrameTilesCopy[item].getQuadcode().c_str());
                event.OptionalPayload = nullptr;
                events.push_back(event);
                continue;
            }

            if (currentFrameTiles.find(item) != std::end(currentFrameTiles)) {
                MapEvent event{};
                event.Type = InFrustum;
                strcpy_s(event.Quadcode, mCurrentMetaTiles[item].getQuadcode().c_str());
                event.OptionalPayload = (void *) &mCurrentMetaTiles[item].mPayload;
                events.push_back(event);
                continue;
            }
        }

        mStoredMetaEvents = events;

        populateRenderingQueue();

        return events;
    }

    std::vector<MapEvent> MapCore::getContentFrameEvents() {
        std::lock_guard lock{mEventsLock};

        auto events = mStoredContentEvents;
        mStoredContentEvents.clear();

        return events;
    }

    void MapCore::pushEventToContentEvent(const MapEvent &event) {
        std::lock_guard lock{mEventsLock};
        mStoredContentEvents.push_back(event);
    };

    void MapCore::populateRenderingQueue() {
        auto tiles = ((TerrainedWorld *) mWorld)->getMetaTiles();

        for (const auto &item: mStoredMetaEvents)
            mRenderingContext.pushTaskToQueue(new RenderingTask{
                    this, &mDataStash, std::string{item.Quadcode}
            });
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
        mCommonTiles.clear();

        for (const auto& item : get_tiles())
            mCommonTiles.push_back(*item);

        return std::move(mCommonTiles);
    }
    const std::vector<TileDescription>& map_core::emscripten_get_meta_tiles() {
        mMetaTiles.clear();

        for (const auto& item : get_height_tiles())
            mMetaTiles.push_back(*item);

        return std::move(mMetaTiles);
    }
#endif

    DllExport KCore::MapCore *InstantiateMapCore(float latitude, float longitude) {
        return new KCore::MapCore(latitude, longitude);
    }

    DllExport void UpdateMapCore(KCore::MapCore *mapCore,
                                 float *cameraProjectionMatrix,
                                 float *cameraViewMatrix,
                                 float *cameraPosition) {
        mapCore->update(cameraProjectionMatrix,
                        cameraViewMatrix,
                        cameraPosition);
    }

    DllExport KCore::MapEvent *GetCommonFrameEvents(KCore::MapCore *mapCore, int &length) {
        mapCore->mStoredCommonEvents = mapCore->getCommonFrameEvents();
        length = (int) mapCore->mStoredCommonEvents.size();
        return mapCore->mStoredCommonEvents.data();
    }

    DllExport KCore::MapEvent *GetMetaFrameEvents(KCore::MapCore *mapCore, int &length) {
        mapCore->mStoredMetaEvents = mapCore->getMetaFrameEvents();
        length = (int) mapCore->mStoredMetaEvents.size();
        return mapCore->mStoredMetaEvents.data();
    }

    DllExport KCore::MapEvent *GetContentFrameEvents(KCore::MapCore *mapCore, int &length) {
        length = (int) mapCore->mStoredContentEvents.size();
        return mapCore->mStoredContentEvents.data();
    }
}
