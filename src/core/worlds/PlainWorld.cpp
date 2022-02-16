#include "PlainWorld.hpp"
#include "../misc/STBImageUtils.hpp"
#include "../sources/local/geojson/GeoJSONObject.hpp"
#include "../sources/local/geojson/primitives/GeoJSONTransObject.hpp"
#include "../meshes/PolylineMesh.hpp"
#include "../meshes/PolygonMesh.hpp"
#include "../sources/RemoteSource.hpp"

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
                auto url = ((RemoteSource *) mSources["base"])->bakeUrl(mCurrBaseTiles[item]);
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
                            auto *result = (std::vector<GeoJSONObject> *) mSources["json"]->getDataForTile(zoom, x, y);

                            auto size = result->size();
                            auto *objects = new std::vector<GeoJSONTransObject>();

                            for (int i = 0; i < size; i++) {
                                auto &ref = (*result)[i];
                                GeoJSONTransObject obj{
                                        ref.mType,
                                        (int) (ref.mMainShapeCoords.size()),
                                        (int) (ref.mHoleShapeCoords.size()),
                                        nullptr, nullptr
                                };

                                auto convertedMain = std::vector<std::array<double, 2>>{};
                                auto convertedHole = std::vector<std::array<double, 2>>{};

                                if (obj.mainShapeCoordsCount > 0) {
                                    obj.mainShapePositions = new glm::vec3[obj.mainShapeCoordsCount];

                                    for (int idx = 0; idx < obj.mainShapeCoordsCount; idx++) {
                                        auto project = latLonToWorldPosition(
                                                {ref.mMainShapeCoords[idx][1], ref.mMainShapeCoords[idx][0]
                                                });
                                        convertedMain.push_back({project.x, project.y});
                                        obj.mainShapePositions[idx] = {project.x, 0.0f, project.y};
                                    }
                                }

                                if (obj.holeShapeCoordsCount > 0) {
                                    auto coordsCount = obj.holeShapeCoordsCount;
                                    obj.holeShapePositions = new glm::vec3[coordsCount];

                                    for (int idx = 0; idx < coordsCount; idx++) {
                                        auto project = latLonToWorldPosition(
                                                {ref.mHoleShapeCoords[idx][1], ref.mHoleShapeCoords[idx][0]
                                                });
                                        convertedHole.push_back({project.x, project.y});
                                        obj.holeShapePositions[idx] = {project.x, 0.0f, project.y};
                                    }
                                }

                                if (ref.mType == Polyline)
                                    obj.mesh = new PolylineMesh(ref, convertedMain);


                                if (ref.mType == Polygon || ref.mType == PolygonWithHole)
                                    obj.mesh = new PolygonMesh(ref, convertedMain, convertedHole);

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