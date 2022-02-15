#include "MapCore.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "worlds/PlainWorld.hpp"
#include "queue/tasks/CallbackTask.hpp"
#include "misc/Utils.hpp"
#include "misc/STBImageUtils.hpp"
#include "meshes/PolylineMesh.hpp"
#include "meshes/PolygonMesh.hpp"

namespace KCore {
    MapCore::MapCore() {
        mDataStash.setMaximalCount(1000);
        mDataStash.setStayAliveInterval(3);
    }

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

//        if (!mWorldAdapter) throw std::runtime_error("world not initialized");
//
//        auto previousFrameTilesCopy = mCurrentCommonTiles;
//
//        auto tiles = mWorldAdapter->getTiles();
//        std::map<std::string, TileDescription> currentFrameTiles;
//        for (const auto &item: tiles)
//            currentFrameTiles[item.getQuadcode()] = item;
//
//        mCurrentCommonTiles = currentFrameTiles;
//
//        auto diff = mapKeysDifference<std::string>(previousFrameTilesCopy, currentFrameTiles);
//        auto inter = mapKeysIntersection<std::string>(previousFrameTilesCopy, currentFrameTiles);
//
//        auto events = std::vector<MapEvent>();
//        for (auto &item: diff) {
//            bool inPrev = previousFrameTilesCopy.find(item) != std::end(previousFrameTilesCopy);
//            bool inNew = currentFrameTiles.find(item) != std::end(currentFrameTiles);
//
//            if (inPrev) {
//                events.push_back(MapEvent::MakeNotInFrustumEvent(item));
//                continue;
//            }
//
//            if (inNew) {
//                events.push_back(MapEvent::MakeInFrustumEvent(item, &mCurrentCommonTiles[item].mPayload));
//
//                // ... so, download resource for appeared element
//                auto composite = std::string{item} + ".common.image";
//                auto test = mDataStash.getByKey(composite);
//                auto inStash = test != nullptr;
//                if (inStash)
//                    pushEventToContentQueue(MapEvent::MakeImageLoadedEvent(item, test->get()));
//
////                std::string token = "pk.eyJ1IjoiYW5rYW5vIiwiYSI6ImNqeWVocmNnYTAxaWIzaGxoeGd4ejExN3MifQ.8QQWwjxjyoIH8ma0McKeNA";
////                std::string url{"http://api.mapbox.com/v4/mapbox.satellite/" + mCurrentCommonTiles[item].tileURL() +
////                                 ".png?access_token=" + token};
//
//                std::string url{"http://tile.openstreetmap.org/" + mCurrentCommonTiles[item].tileURL() + ".png"};
//                std::string quadcode{mCurrentCommonTiles[item].getQuadcode()};
//                std::string tag{"common.image"};
//
//                auto request = new NetworkRequest{
//                        url,
//                        [url, tag, this, composite](const std::vector<uint8_t> &data) {
//                            auto raw = std::make_shared<std::vector<uint8_t>>(
//                                    KCore::STBImageUtils::decodeImageBuffer(data));
//                            mDataStash.setOrReplace(composite, raw);
////                            mRenderingContext.pushTextureDataToGPUQueue(composite, raw);
//                            std::cout << url << " finally loaded kek" << std::endl;
//                        }, nullptr
//                };
//                mWorldAdapter->getNetworkContext().pushRequestToQueue(request);
//
//                auto tile = mCurrentCommonTiles[item];
//                mWorldAdapter->getTaskContext().pushTaskToQueue(new CallbackTask{
//                        [tile, this]() {
//                            auto tilecode = tile.getTilecode();
//                            auto zoom = tilecode.z, x = tilecode.x, y = tilecode.y;
//
//                            auto *result = (std::vector<GeoJSONObject> *) primitivesSource.getDataForTile(zoom, x, y);
//
//                            auto size = result->size();
//                            auto *objects = new std::vector<GeoJSONTransObject>();
//
//                            for (int i = 0; i < size; i++) {
//                                auto &ref = (*result)[i];
//
//                                GeoJSONTransObject obj{};
//                                obj.type = ref.mType;
//                                obj.mainShapeCoordsCount = ref.mMainShapeCoords.size();
//                                obj.holeShapeCoordsCount = ref.mHoleShapeCoords.size();
//                                obj.holeShapePositions = nullptr;
//                                obj.mainShapePositions = nullptr;
//
//                                if (ref.mType == Polyline) {
//                                    auto converted = std::vector<std::array<double, 2>>{};
//                                    for (const auto &item: ref.mMainShapeCoords) {
//                                        auto project = mWorldAdapter->latLonToWorldPosition(
//                                                {item[1], item[0]}
//                                        );
//                                        converted.push_back({project.x, project.y});
//                                    }
//
//                                    obj.mesh = new PolylineMesh(ref, converted);
//                                }
//
//                                if (ref.mType == Polygon || ref.mType == PolygonWithHole) {
//                                    auto convertedMain = std::vector<std::array<double, 2>>{};
//                                    for (const auto &item: ref.mMainShapeCoords) {
//                                        auto project = mWorldAdapter->latLonToWorldPosition(
//                                                {item[1], item[0]}
//                                        );
//                                        convertedMain.push_back({project.x, project.y});
//                                    }
//
//                                    auto convertedHole = std::vector<std::array<double, 2>>{};
//                                    for (const auto &item: ref.mHoleShapeCoords) {
//                                        auto project = mWorldAdapter->latLonToWorldPosition(
//                                                {item[1], item[0]}
//                                        );
//                                        convertedHole.push_back({project.x, project.y});
//                                    }
//
//                                    obj.mesh = new PolygonMesh(ref,
//                                                               convertedMain,
//                                                               convertedHole);
//                                }
//
//                                if (obj.mainShapeCoordsCount) {
//                                    obj.mainShapePositions = new glm::vec3[obj.mainShapeCoordsCount];
//                                    for (int j = 0; j < obj.mainShapeCoordsCount; j++) {
//                                        auto project = mWorldAdapter->latLonToWorldPosition(
//                                                {ref.mMainShapeCoords[j][1], ref.mMainShapeCoords[j][0]}
//                                        );
//                                        obj.mainShapePositions[j] = {project.x, 0.0f, project.y};
//                                    }
//                                }
//
//                                if (obj.holeShapeCoordsCount) {
//                                    obj.holeShapePositions = new glm::vec3[obj.holeShapeCoordsCount];
//                                    for (int j = 0; j < obj.holeShapeCoordsCount; j++) {
//                                        auto project = mWorldAdapter->latLonToWorldPosition(
//                                                {ref.mHoleShapeCoords[j][1], ref.mHoleShapeCoords[j][0]}
//                                        );
//                                        obj.holeShapePositions[j] = {project.x, 0.0f, project.y};
//                                    }
//                                }
//
//                                objects->push_back(obj);
//                            }
//
//                            delete result;
//
//                            auto composite = tile.getQuadcode() + ".common.geojson";
//                            if (size > 0) {
//                                auto ptr = std::shared_ptr<std::vector<GeoJSONTransObject>>();
//                                ptr.reset(objects);
//                                mDataStash.setOrReplace(composite, ptr);
//
//                                auto event = MapEvent::MakeGeoJSONEvent(tile.getQuadcode(), ptr.get());
//                                pushEventToContentQueue(event);
//                            } else {
//                                mDataStash.setOrReplace(composite, nullptr);
//                            }
//                        }
//                });
//                continue;
//            }
//        }
//
//        mSyncEvents = events;

        return mWorldAdapter->getSyncEvents();
    }

    std::vector<MapEvent> MapCore::getContentFrameEvents() {
//        std::lock_guard lock{mEventsLock};
//
//        auto events = mActualContentEvents;
//        mActualContentEvents.clear();

        return {};
    }

    void MapCore::pushEventToContentQueue(const MapEvent &event) {
//        std::lock_guard lock{mEventsLock};
//        mActualContentEvents.push_back(event);
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

    DllExport void SetWorldAdapter(KCore::MapCore *core, KCore::BaseWorld* adapter) {
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

        auto* events = new MapEvent[length];
        std::copy(syncEvents.begin(), syncEvents.end(), events);

        return events;
    }

    DllExport void ReleaseSyncEvents(MapEvent* syncArrayPtr) {
        delete[] syncArrayPtr;
    }

    DllExport void *GetBufferPtrFromTag(KCore::MapCore *mapCore, const char *tag, int &length) {
        auto stash = &mapCore->mDataStash;

        auto buffer = (std::shared_ptr<std::vector<uint8_t>> *) stash->getByKey(tag);
        auto &buffer_ref = *buffer;
        length = (int) buffer_ref->size();
        return buffer_ref->data();
    }

    DllExport void *GetPoints(std::vector<GeoJSONTransObject> *points, int &length) {
        if (points == nullptr)
            length = 0;
        else
            length = points->size();

        return points->data();
    }
}