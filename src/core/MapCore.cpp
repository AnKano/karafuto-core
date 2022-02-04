#include <glm/gtc/type_ptr.hpp>

#include "MapCore.hpp"

#include "sources/remote/RasterRemoteSource.hpp"
#include "worlds/PlainWorld.hpp"
#include "queue/tasks/CallbackTask.hpp"
#include "misc/Utils.hpp"
#include "misc/STBImageUtils.hpp"

namespace KCore {
    MapCore::MapCore(float latitude, float longitude) {
        glm::vec2 origin_lat_lon{latitude, longitude};
        glm::vec2 origin_point{GeographyConverter::latLonToPoint(origin_lat_lon)};

        KCore::WorldConfig config{};
        config.GenerateMeta = true;

        mWorld = new TerrainedWorld(origin_lat_lon, origin_point, config);
        mWorld->updateFrustum(this->mCameraProjectionMatrix, this->mCameraViewMatrix);
        mWorld->setPosition(this->mCameraPosition);

        mDataStash.setMaximalCount(1000);
        mDataStash.setStayAliveInterval(3);
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
            bool inPrev = previousFrameTilesCopy.find(item) != std::end(previousFrameTilesCopy);
            bool inNew = currentFrameTiles.find(item) != std::end(currentFrameTiles);

            if (inPrev) {
                events.push_back(MapEvent::MakeNotInFrustumEvent(item));
                continue;
            }

            if (inNew) {
                events.push_back(MapEvent::MakeInFrustumEvent(item, &mCurrentCommonTiles[item].mPayload));

                // ... so, download resource for appeared element
                auto composite = std::string{item} + ".common.image";
                auto test = mDataStash.getByKey(composite);
                auto inStash = test != nullptr;
                if (inStash)
                    pushEventToContentQueue(MapEvent::MakeImageLoadedEvent(item, test->get()));

//                std::string token = "pk.eyJ1IjoiYW5rYW5vIiwiYSI6ImNqeWVocmNnYTAxaWIzaGxoeGd4ejExN3MifQ.8QQWwjxjyoIH8ma0McKeNA";
//                std::string url{"http://api.mapbox.com/v4/mapbox.satellite/" + mCurrentCommonTiles[item].tileURL() +
//                                 ".png?access_token=" + token};

                std::string url{"http://tile.openstreetmap.org/" + mCurrentCommonTiles[item].tileURL() + ".png"};
                std::string quadcode{mCurrentCommonTiles[item].getQuadcode()};
                std::string tag{"common.image"};

                auto request = new NetworkRequest{
                        url,
                        [url, tag, this, composite](const std::vector<uint8_t> &data) {
                            auto raw = std::make_shared<std::vector<uint8_t>>(
                                    KCore::STBImageUtils::decodeImageBuffer(data));
                            mDataStash.setOrReplace(composite, raw);
                            mRenderingContext.pushTextureDataToGPUQueue(composite, raw);
                            std::cout << url << " finally loaded kek" << std::endl;
                        },
                        [url]() {
                            std::cout << url << " not loaded" << std::endl;
                        }
                };
                mNetworkingContext.pushRequestToQueue(request);
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
            currentFrameTiles[item.description.getQuadcode()] = item.description;

        mCurrentMetaTiles = currentFrameTiles;

        auto diff = mapKeysDifference<std::string>(previousFrameTilesCopy, currentFrameTiles);
        auto inter = mapKeysIntersection<std::string>(previousFrameTilesCopy, currentFrameTiles);

        auto events = std::vector<MapEvent>();
        for (const auto &item: diff) {
            bool inPrev = previousFrameTilesCopy.find(item) != std::end(previousFrameTilesCopy);
            bool inNew = currentFrameTiles.find(item) != std::end(currentFrameTiles);

            if (inPrev) {
                events.push_back(MapEvent::MakeNotInFrustumEvent(item));
                continue;
            }

            if (inNew) {
                events.push_back(MapEvent::MakeInFrustumEvent(item, &mCurrentMetaTiles[item].mPayload));
                continue;
            }
        }

        mStoredMetaEvents = events;

        if (!diff.empty()) {
            std::cout << mRenderingContext.mQueue.mQueue.size() << std::endl;
            mRenderingContext.clearQueue();

            for (const auto &item: tiles)
                mRenderingContext.pushTaskToQueue(new RenderTask{
                        this, item.description.getQuadcode(),
                        item.childQuadcodes, item.parentQuadcodes
                });
        }

        return events;
    }

    std::vector<MapEvent> MapCore::getContentFrameEvents() {
        std::lock_guard lock{mEventsLock};

        auto events = mActualContentEvents;
        mActualContentEvents.clear();

        return events;
    }

    void MapCore::pushEventToContentQueue(const MapEvent &event) {
        std::lock_guard lock{mEventsLock};
        mActualContentEvents.push_back(event);
    };

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
        mapCore->mStoredContentEvents = mapCore->getContentFrameEvents();
        length = (int) mapCore->mStoredContentEvents.size();
        return mapCore->mStoredContentEvents.data();
    }

    DllExport void *GetBufferPtrFromTag(KCore::MapCore *mapCore, const char *tag, int &length) {
        auto stash = &mapCore->mDataStash;

        auto buffer = (std::shared_ptr<std::vector<uint8_t>> *) stash->getByKey(tag);
        auto &buffer_ref = *buffer;
        length = (int) buffer_ref->size();
        return buffer_ref->data();
    }

    DllExport void ReleaseCopy(const uint8_t *ptr) {
        delete ptr;
    }
}