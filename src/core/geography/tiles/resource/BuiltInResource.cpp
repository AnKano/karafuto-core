//#include "BuiltInResource.hpp"
//
//#include "../GenericTile.hpp"
//#include "../../../sources/RemoteSource.hpp"
//#include "../../../sources/local/srtm/SRTMLocalSource.hpp"
//
//#include "../../../contexts/network/NetworkRequest.hpp"
//#include "../../../worlds/IWorld.hpp"
//#include "../../../meshes/GridMesh.hpp"
//#include "../../../worlds/TerrainedWorld.hpp"
//
//#include <future>
//
//namespace KCore {
//    std::function<void(KCore::IWorld *, GenericTile *tile)> BuiltInResource::ImageCalculate() {
//        auto processor = [](IWorld *world, GenericTile *tile) {
//            auto desc = tile->getTileDescription();
//            auto url = ((RemoteSource *) world->getSources()["base"])->bakeUrl(desc);
//            auto request = new KCore::NetworkRequest{
//                    url,
//                    [world, desc](const std::vector<uint8_t> &data) {
//                        int width = -1, height = -1, channels = -1;
//                        auto image = STBImageUtils::decodeImageBuffer(
//                                data.data(), data.size(), width, height, channels
//                        );
//
//                        auto raw = new uint8_t[image.size()];
//                        std::copy(image.begin(), image.end(), raw);
//
//                        auto event = KCore::Event::MakeImageLoadedEvent(desc.getQuadcode(), raw);
//                        world->pushToAsyncEvents(event);
//                    }, nullptr
//            };
//            world->getNetworkContext()->pushRequestToQueue(request);
//
//            tile->commitTag("image");
//        };
//        return processor;
//    }
//
//    std::function<void(KCore::IWorld *, GenericTile *tile)> BuiltInResource::ImageCalculateMeta() {
//        auto processor = [](IWorld *world, GenericTile *tile) {
//            auto desc = tile->getTileDescription();
//            auto url = ((RemoteSource *) world->getSources()["base"])->bakeUrl(desc);
//            auto request = new KCore::NetworkRequest{
//                    url,
//                    [world, desc](const std::vector<uint8_t> &data) {
//                        auto convWorld = (TerrainedWorld *) world;
//
//                        // but anyway store raw data (we will compress and use it after)
//                        convWorld->getRenderContext()->storeTextureInContext(data, desc.getQuadcode());
//                    }, nullptr
//            };
//            world->getNetworkContext()->pushRequestToQueue(request);
//
//            tile->commitTag("image");
//        };
//        return processor;
//    }
//
//    std::function<void(KCore::IWorld *, GenericTile *tile)> BuiltInResource::JSONCalculate() {
//        auto processor = [](IWorld *world, GenericTile *tile) {
//            auto desc = tile->getTileDescription();
//            auto task = new KCore::CallbackTask{
//                    [world, desc]() {
//
//                        auto tilecode = desc.getTilecode();
//                        auto zoom = tilecode.z, x = tilecode.x, y = tilecode.y;
//                        auto *result = (std::vector<KCore::GeoJSONObject> *)
//                                world->getSources()["json"]->getDataForTile(zoom, x, y);
//                        auto size = result->size();
//                        auto *objects = new std::vector<KCore::GeoJSONTransObject>();
//                        for (std::size_t i = 0; i < size; i++) {
//                            auto &ref = (*result)[i];
//                            KCore::GeoJSONTransObject obj{
//                                    ref.mType,
//                                    (int) (ref.mMainShapeCoords.size()),
//                                    (int) (ref.mHoleShapeCoords.size()),
//                                    nullptr, nullptr
//                            };
//                            auto convertedMain = std::vector<std::array<double, 2>>{};
//                            auto convertedHole = std::vector<std::array<double, 2>>{};
//                            if (obj.mainShapeCoordsCount > 0) {
//                                obj.mainShapePositions = new glm::vec3[obj.mainShapeCoordsCount];
//                                for (int idx = 0; idx < obj.mainShapeCoordsCount; idx++) {
//                                    auto project = world->latLonToWorldPosition({
//                                                                                        ref.mMainShapeCoords[idx][1],
//                                                                                        ref.mMainShapeCoords[idx][0]
//                                                                                });
//                                    convertedMain.push_back({project.x, project.y});
//                                    obj.mainShapePositions[idx] = {project.x, 0.0f, project.y};
//                                }
//                            }
//                            if (obj.holeShapeCoordsCount > 0) {
//                                auto coordsCount = obj.holeShapeCoordsCount;
//                                obj.holeShapePositions = new glm::vec3[coordsCount];
//                                for (int idx = 0; idx < coordsCount; idx++) {
//                                    auto project = world->latLonToWorldPosition(
//                                            {ref.mHoleShapeCoords[idx][1],
//                                             ref.mHoleShapeCoords[idx][0]
//                                            });
//                                    convertedHole.push_back({project.x, project.y});
//                                    obj.holeShapePositions[idx] = {project.x, 0.0f, project.y};
//                                }
//                            }
//                            if (ref.mType == KCore::Polyline)
//                                obj.mesh = new KCore::PolylineMesh(ref, convertedMain);
//                            if (ref.mType == KCore::Polygon || ref.mType == KCore::PolygonWithHole)
//                                obj.mesh = new KCore::PolygonMesh(ref, convertedMain, convertedHole);
//                            objects->push_back(obj);
//                        }
//                        delete result;
//                        if (objects->empty()) return;
//                        auto event = KCore::Event::MakeGeoJSONEvent(desc.getQuadcode(), objects);
//                        world->pushToAsyncEvents(event);
//
//                    }
//            };
//            world->getTaskContext().pushTaskToQueue(task);
//        };
//        return processor;
//    }
//
//    std::function<void(KCore::IWorld *, GenericTile *tile)> BuiltInResource::TerrainCalculate() {
//        auto processor = [](IWorld *world, GenericTile *tile) {
//            auto desc = tile->getTileDescription();
//            world->getTaskContext().pushTaskToQueue(new CallbackTask{
//                    [world, desc, tile]() {
//                        auto tilecode = desc.getTilecode();
//                        auto zoom = tilecode.z, x = tilecode.x, y = tilecode.y;
//
//                        auto *source = (KCore::SRTMLocalSource *) world->getSources()["terrain"];
//                        auto result = source->getDataForTile(zoom, x, y, 128, 128);
//
//                        auto mesh = new GridMesh(1.0f, 1.0f, 127, 127, result);
//
//                        delete[] result;
//
//                        auto event = Event::MakeTerrainEvent(desc.getQuadcode(), mesh);
//                        world->pushToAsyncEvents(event);
//
//                        tile->commitTag("terrain");
//                    }
//            });
//        };
//        return processor;
//    }
//
//    std::function<void(KCore::IWorld *, GenericTile *tile)> BuiltInResource::JSONWithTerrainAdaptation() {
//        auto processor = [](IWorld *world, GenericTile *tile) {
//            auto desc = tile->getTileDescription();
//            auto task = new KCore::CallbackTask{
//                    [world, desc]() {
//                        auto tilecode = desc.getTilecode();
//                        auto zoom = tilecode.z, x = tilecode.x, y = tilecode.y;
//
//                        auto *terrainSource = (KCore::SRTMLocalSource *) world->getSources()["terrain"];
//                        auto *result = (std::vector<KCore::GeoJSONObject> *)
//                                world->getSources()["json"]->getDataForTile(zoom, x, y);
//                        auto *objects = new std::vector<KCore::GeoJSONTransObject>();
//                        for (auto &ref: *result) {
//                            KCore::GeoJSONTransObject obj{
//                                    ref.mType,
//                                    (int) (ref.mMainShapeCoords.size()),
//                                    (int) (ref.mHoleShapeCoords.size()),
//                                    nullptr, nullptr,
//                                    nullptr, 0.0f
//                            };
//
//                            auto height = FLT_MAX;
//                            for (const auto &item: ref.mMainShapeCoords) {
//                                auto res = terrainSource->getElevationForLatLonPoint(item[0], item[1]);
//                                if (res < height) height = res;
//                            }
//                            obj.height = height;
//
//                            auto convertedMain = std::vector<std::array<double, 2>>{};
//                            auto convertedHole = std::vector<std::array<double, 2>>{};
//
//                            if (obj.mainShapeCoordsCount > 0) {
//                                obj.mainShapePositions = new glm::vec3[obj.mainShapeCoordsCount];
//                                for (int idx = 0; idx < obj.mainShapeCoordsCount; idx++) {
//                                    auto project = world->latLonToWorldPosition(
//                                            {ref.mMainShapeCoords[idx][1], ref.mMainShapeCoords[idx][0]}
//                                    );
//                                    convertedMain.push_back({project.x, project.y});
//                                    obj.mainShapePositions[idx] = {project.x, 0.0f, project.y};
//                                }
//                            }
//                            if (obj.holeShapeCoordsCount > 0) {
//                                auto coordsCount = obj.holeShapeCoordsCount;
//                                obj.holeShapePositions = new glm::vec3[coordsCount];
//                                for (int idx = 0; idx < coordsCount; idx++) {
//                                    auto project = world->latLonToWorldPosition(
//                                            {ref.mHoleShapeCoords[idx][1], ref.mHoleShapeCoords[idx][0]}
//                                    );
//                                    convertedHole.push_back({project.x, project.y});
//                                    obj.holeShapePositions[idx] = {project.x, 0.0f, project.y};
//                                }
//                            }
//
//                            if (ref.mType == KCore::Polyline) obj.mesh = new KCore::PolylineMesh(ref, convertedMain);
//                            if (ref.mType == KCore::Polygon || ref.mType == KCore::PolygonWithHole)
//                                obj.mesh = new KCore::PolygonMesh(ref, convertedMain, convertedHole);
//                            objects->push_back(obj);
//                        }
//
//                        delete result;
//
//                        if (objects->empty()) return;
//
//                        auto event = KCore::Event::MakeGeoJSONEvent(
//                                desc.getQuadcode(), objects
//                        );
//                        world->pushToAsyncEvents(event);
//
//                    }
//            };
//            world->getTaskContext().pushTaskToQueue(task);
//        };
//        return processor;
//    }
//}