#include "BuiltinStages.hpp"

#include "Stage.hpp"
#include "../BaseWorld.hpp"
#include "../../sources/RemoteSource.hpp"
#include "../../misc/STBImageUtils.hpp"
#include "../../sources/local/geojson/GeoJSONObject.hpp"
#include "../../sources/local/geojson/primitives/GeoJSONTransObject.hpp"
#include "../../meshes/PolylineMesh.hpp"
#include "../../meshes/PolygonMesh.hpp"

namespace KCore {
    KCore::Stage *BuiltInStages::CommonCalculate() {
        auto stage = new KCore::Stage([](KCore::BaseWorld *world) {
            auto currTiles = world->getCurrentBaseTiles();
            auto prevTiles = world->getPreviousBaseTiles();

            auto diff = mapKeysDifference<std::string>(currTiles, prevTiles);
            auto inter = mapKeysIntersection<std::string>(currTiles, prevTiles);

            for (auto &item: diff) {
                bool inPrev = prevTiles.count(item) > 0;
                bool inNew = currTiles.count(item) > 0;

                if (inPrev) {
                    auto event = KCore::MapEvent::MakeNotInFrustumEvent(item);
                    world->pushToSyncEvents(event);
                }

                if (inNew) {
                    auto event = KCore::MapEvent::MakeInFrustumEvent(item, &currTiles[item].mPayload);
                    world->pushToSyncEvents(event);
                }
            }
        });

        return stage;
    }

    KCore::Stage *BuiltInStages::ImageCalculate() {
        auto imageGen = new KCore::Stage([](KCore::BaseWorld *world) {
            auto currTiles = world->getCurrentBaseTiles();
            auto prevTiles = world->getPreviousBaseTiles();

            auto diff = mapKeysDifference<std::string>(currTiles, prevTiles);
            auto inter = mapKeysIntersection<std::string>(currTiles, prevTiles);

            for (auto &item: diff) {
                bool inNew = currTiles.count(item) > 0;

                if (inNew) {
                    auto url = ((KCore::RemoteSource *) world->getSources()["base"])->bakeUrl(currTiles[item]);
                    auto request = new KCore::NetworkRequest{
                            url,
                            [world, item](const std::vector<uint8_t> &data) {
                                auto image = KCore::STBImageUtils::decodeImageBuffer(data);

                                auto raw = new uint8_t[image.size()];
                                std::copy(image.begin(), image.end(), raw);

                                auto event = KCore::MapEvent::MakeImageLoadedEvent(
                                        item, raw
                                );
                                world->pushToAsyncEvents(event);
                            }, nullptr
                    };
                    world->getNetworkContext().pushRequestToQueue(request);
                }
            }
        });

        return imageGen;
    }

    KCore::Stage *BuiltInStages::JSONCalculate() {
        auto jsonGen = new KCore::Stage([](KCore::BaseWorld *world) {
            auto currTiles = world->getCurrentBaseTiles();
            auto prevTiles = world->getPreviousBaseTiles();

            auto diff = mapKeysDifference<std::string>(currTiles, prevTiles);
            auto inter = mapKeysIntersection<std::string>(currTiles, prevTiles);

            for (auto &item: diff) {
                bool inNew = currTiles.count(item) > 0;

                if (inNew) {
                    auto tile = currTiles[item];

                    auto task = new KCore::CallbackTask{
                            [world, tile]() {
                                auto tilecode = tile.getTilecode();
                                auto zoom = tilecode.z, x = tilecode.x, y = tilecode.y;
                                auto *result = (std::vector<KCore::GeoJSONObject> *) world->getSources()["json"]->getDataForTile(
                                        zoom, x, y);

                                auto size = result->size();
                                auto *objects = new std::vector<KCore::GeoJSONTransObject>();

                                for (int i = 0; i < size; i++) {
                                    auto &ref = (*result)[i];
                                    KCore::GeoJSONTransObject obj{
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
                                            auto project = world->latLonToWorldPosition(
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
                                            auto project = world->latLonToWorldPosition(
                                                    {ref.mHoleShapeCoords[idx][1], ref.mHoleShapeCoords[idx][0]
                                                    });
                                            convertedHole.push_back({project.x, project.y});
                                            obj.holeShapePositions[idx] = {project.x, 0.0f, project.y};
                                        }
                                    }

                                    if (ref.mType == KCore::Polyline)
                                        obj.mesh = new KCore::PolylineMesh(ref, convertedMain);


                                    if (ref.mType == KCore::Polygon || ref.mType == KCore::PolygonWithHole)
                                        obj.mesh = new KCore::PolygonMesh(ref, convertedMain, convertedHole);

                                    objects->push_back(obj);
                                }

                                delete result;

                                if (objects->empty()) return;

                                auto event = KCore::MapEvent::MakeGeoJSONEvent(tile.getQuadcode(), objects);
                                world->pushToAsyncEvents(event);
                            }
                    };

                    world->getTaskContext().pushTaskToQueue(task);
                }
            }
        });
        return jsonGen;
    }
}