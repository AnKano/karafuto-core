#include "PlainWorld.hpp"
#include "../misc/STBImageUtils.hpp"
#include "../sources/local/geojson/GeoJSONObject.hpp"
#include "../sources/local/geojson/primitives/GeoJSONTransObject.hpp"
#include "../meshes/PolylineMesh.hpp"
#include "../meshes/PolygonMesh.hpp"

namespace KCore {
    void PlainWorld::calculateTiles() {
        BaseWorld::calculateTiles();
    }

    void PlainWorld::makeEvents() {
        mSyncEvents.clear();

        auto diff = mapKeysDifference<std::string>(mPrevBaseTiles, mCurrBaseTiles);
        auto inter = mapKeysIntersection<std::string>(mPrevBaseTiles, mCurrBaseTiles);

        for (auto &item: diff) {
            bool inPrev = mPrevBaseTiles.count(item) > 0;
            bool inNew = mCurrBaseTiles.count(item) > 0;

            if (inPrev) {
                auto event = MapEvent::MakeNotInFrustumEvent(item);
                pushToSyncEvents(event);
            }

            if (inNew) {
                auto event = MapEvent::MakeInFrustumEvent(item, &mCurrBaseTiles[item].mPayload);
                pushToSyncEvents(event);
            }
        }

        for (auto &item: diff) {
            bool inNew = mCurrBaseTiles.count(item) > 0;

            if (inNew) {
                std::string url{"http://tile.openstreetmap.org/" + mCurrBaseTiles[item].tileURL() + ".png"};
                auto request = new NetworkRequest{
                        url,
                        [this, item](const std::vector<uint8_t> &data) {
                            auto image = KCore::STBImageUtils::decodeImageBuffer(data);

                            auto raw = new uint8_t[image.size()];
                            std::copy(image.begin(), image.end(), raw);

                            auto event = MapEvent::MakeImageLoadedEvent(
                                    item, raw
                            );
                            pushToAsyncEvents(event);
                        }, nullptr
                };
                mNetworkContext.pushRequestToQueue(request);
            }
        }

        for (auto &item: diff) {
            bool inNew = mCurrBaseTiles.count(item) > 0;

            if (inNew) {
                auto tile = mCurrBaseTiles[item];

                auto task = new CallbackTask{
                        [this, tile]() {
                            auto tilecode = tile.getTilecode();
                            auto zoom = tilecode.z, x = tilecode.x, y = tilecode.y;

                            auto *result = (std::vector<GeoJSONObject> *) jsonPrimitives->getDataForTile(zoom, x, y);

                            auto size = result->size();
                            auto *objects = new std::vector<GeoJSONTransObject>();

                            for (int i = 0; i < size; i++) {
                                auto &ref = (*result)[i];
                                GeoJSONTransObject obj{};
                                obj.type = ref.mType;
                                obj.mainShapeCoordsCount = ref.mMainShapeCoords.size();
                                obj.holeShapeCoordsCount = ref.mHoleShapeCoords.size();
                                obj.holeShapePositions = nullptr;
                                obj.mainShapePositions = nullptr;
                                if (ref.mType == Polyline) {
                                    auto converted = std::vector<std::array<double, 2>>{};
                                    for (const auto &item: ref.mMainShapeCoords) {
                                        auto project = latLonToWorldPosition({item[1], item[0]});
                                        converted.push_back({project.x, project.y});
                                    }
                                    obj.mesh = new PolylineMesh(ref, converted);
                                }
                                if (ref.mType == Polygon || ref.mType == PolygonWithHole) {
                                    auto convertedMain = std::vector<std::array<double, 2>>{};
                                    for (const auto &item: ref.mMainShapeCoords) {
                                        auto project = latLonToWorldPosition({item[1], item[0]});
                                        convertedMain.push_back({project.x, project.y});
                                    }
                                    auto convertedHole = std::vector<std::array<double, 2>>{};
                                    for (const auto &item: ref.mHoleShapeCoords) {
                                        auto project = latLonToWorldPosition({item[1], item[0]});
                                        convertedHole.push_back({project.x, project.y});
                                    }
                                    obj.mesh = new PolygonMesh(ref,
                                                               convertedMain,
                                                               convertedHole);
                                }
                                if (obj.mainShapeCoordsCount) {
                                    obj.mainShapePositions = new glm::vec3[obj.mainShapeCoordsCount];
                                    for (int j = 0; j < obj.mainShapeCoordsCount; j++) {
                                        auto project = latLonToWorldPosition(
                                                {ref.mMainShapeCoords[j][1], ref.mMainShapeCoords[j][0]}
                                        );
                                        obj.mainShapePositions[j] = {project.x, 0.0f, project.y};
                                    }
                                }
                                if (obj.holeShapeCoordsCount) {
                                    obj.holeShapePositions = new glm::vec3[obj.holeShapeCoordsCount];
                                    for (int j = 0; j < obj.holeShapeCoordsCount; j++) {
                                        auto project = latLonToWorldPosition(
                                                {ref.mHoleShapeCoords[j][1], ref.mHoleShapeCoords[j][0]}
                                        );
                                        obj.holeShapePositions[j] = {project.x, 0.0f, project.y};
                                    }
                                }
                                objects->push_back(obj);
                            }

                            delete result;

                            if (objects->empty()) return;

                            auto event = MapEvent::MakeGeoJSONEvent(tile.getQuadcode(), objects);
                            pushToAsyncEvents(event);
                        }
                };

                mTaskContext.pushTaskToQueue(task);
            }
        }
    }

    extern "C" {
    DllExport KCore::PlainWorld *CreatePlainWorld(float latitude, float longitude) {
        return new KCore::PlainWorld(latitude, longitude);
    }
    }
}