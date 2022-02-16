////
//// Created by Anton Shubin on 1/18/2021.
////


#include "core/MapCore.hpp"
#include "core/worlds/PlainWorld.hpp"
#include "core/sources/RemoteSource.hpp"
#include "core/misc/STBImageUtils.hpp"
#include "core/meshes/PolylineMesh.hpp"
#include "core/meshes/PolygonMesh.hpp"

#ifdef __EMSCRIPTEN__

#include <emscripten/bind.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(karafuto) {
    register_vector<KCore::TileDescription>("kcore_tile_vector");    register_vector<glm::vec3>("glm_vec3_vector");
    register_vector<glm::vec2>("glm_vec2_vector");
    register_vector<uint32_t>("unsigned_int_vector");
    register_vector<float>("float_vector");
    register_vector<std::string>("string_vector");

    value_object<glm::vec2>("vec2")
            .field("x", &glm::vec2::x)
            .field("y", &glm::vec2::y);

    function("lat_lon_to_point", &KCore::geo_converter::lat_lon_to_point);
    function("point_to_lat_lon", &KCore::geo_converter::point_to_lat_lon);

    class_<KCore::GridMesh>("mesh")
            .constructor<float, float, int>()
            .constructor<float, float, int, int>()

            .function("emscripten_get_indices_ptr", &KCore::GridMesh::emscripten_get_indices_ptr)
            .function("emscripten_get_uvs_ptr", &KCore::GridMesh::emscripten_get_uvs_ptr)
            .function("emscripten_get_normals_ptr", &KCore::GridMesh::emscripten_get_normals_ptr)
            .function("emscripten_get_vertices_ptr", &KCore::GridMesh::emscripten_get_vertices_ptr)
            .function("emscripten_get_constant_ptr", &KCore::GridMesh::emscripten_get_constant_ptr)

            .function("emscripten_get_indices_count", &KCore::GridMesh::emscripten_get_indices_count)
            .function("emscripten_get_uvs_count", &KCore::GridMesh::emscripten_get_uvs_count)
            .function("emscripten_get_normals_count", &KCore::GridMesh::emscripten_get_normals_count)
            .function("emscripten_get_vertices_count", &KCore::GridMesh::emscripten_get_vertices_count)
            .function("emscripten_get_constant_count", &KCore::GridMesh::emscripten_get_constant_count);

    class_<KCore::TileDescription>("TileDescription")
            .property("Quadcode", &KCore::TileDescription::get_quadcode)
            .property("side_length", &KCore::TileDescription::get_side_length);

    class_<KCore::map_core>("map_core")
            .constructor<float, float>()
            .function("update", select_overload<void(intptr_t, intptr_t, intptr_t)>(&KCore::map_core::update))
            .function("get_tiles", &KCore::map_core::get_tiles)
            .function("emscripten_get_tiles", &KCore::map_core::emscripten_get_tiles)
            .function("emscripten_get_meta_tiles", &KCore::map_core::emscripten_get_meta_tiles);
}

#endif

#include <iostream>
#include <chrono>

#include <glm/glm.hpp>

int main() {
    const uint16_t viewportWidth{2560}, viewportHeight{1280};
    const float aspectRatio{(float) viewportWidth / viewportHeight};

    // create camera that describe point of view and matrix
    glm::mat4 cameraProjectionMatrix;
    glm::mat4 cameraViewMatrix;

    glm::vec3 cameraOpenGlSpacePosition{1000.0f, 10000.0f, 10000.0f};
    glm::vec3 cameraOpenGlSpaceTarget{0.0f, 0.0f, 0.0f};
    glm::vec3 cameraOpenGlSpaceUp{0.0f, 1.0f, 0.0f};

    // setup matrices
    {
        cameraViewMatrix = glm::lookAt(
                cameraOpenGlSpacePosition,
                cameraOpenGlSpaceTarget,
                cameraOpenGlSpaceUp
        );

        cameraProjectionMatrix = glm::perspective(
                glm::radians(60.0f), aspectRatio,
                0.1f, 2500000.0f
        );
    }

    const uint16_t iterations{5000};

    // 46.9181f, 142.7189f is latitude and longitude of
    // the surroundings of the city of Yuzhno-Sakhalinsk
    auto *world = new KCore::PlainWorld{46.9181f, 142.7189f};

    auto jsonSource = new KCore::GeoJSONLocalSource;
    jsonSource->addSourcePart("assets/sources/points.geojson");
    world->registerSource(jsonSource, "json");

    auto terrainSource = new KCore::SRTMLocalSource;
    terrainSource->addSourcePart("assets/sources/N45E141.hgt");
    terrainSource->addSourcePart("assets/sources", ".hgt");
    world->registerSource(terrainSource, "terrain");

    auto imageSource = new KCore::RemoteSource("http://tile.openstreetmap.org/{z}/{x}/{y}.png");
    world->registerSource(imageSource, "base");

    auto tileGen = new KCore::Stage([](KCore::BaseWorld *world) {
        auto &currTiles = world->getCurrentBaseTiles();
        auto &prevTiles = world->getPreviousBaseTiles();

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
    world->registerStage(tileGen);

    auto imageGen = new KCore::Stage([](KCore::BaseWorld *world) {
        auto &currTiles = world->getCurrentBaseTiles();
        auto &prevTiles = world->getPreviousBaseTiles();

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
    world->registerStage(imageGen);

    auto jsonGen = new KCore::Stage([](KCore::BaseWorld *world) {
        auto &currTiles = world->getCurrentBaseTiles();
        auto &prevTiles = world->getPreviousBaseTiles();

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
    world->registerStage(jsonGen);

    KCore::MapCore core;
    core.setWorldAdapter(world);

    auto start = std::chrono::system_clock::now();
    for (auto i = 0; i < iterations; i++) {
        core.update(cameraProjectionMatrix, cameraViewMatrix, cameraOpenGlSpacePosition);
        auto a = core.getSyncEvents();
        auto b = core.getAsyncEvents();
    }
    auto elapsed = std::chrono::system_clock::now() - start;

    std::cout << (elapsed / std::chrono::microseconds(1)) / iterations
              << " microseconds per iterations" << std::endl;

    return 0;
}