////
//// Created by Anton Shubin on 1/18/2021.
////


#include "core/MapCore.hpp"
#include "core/worlds/PlainWorld.hpp"
#include "core/sources/RemoteSource.hpp"
#include "core/meshes/PolylineMesh.hpp"

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

    const uint16_t iterations{50000};

    // 46.9181f, 142.7189f is latitude and longitude of
    // the surroundings of the city of Yuzhno-Sakhalinsk
//    auto *world = new KCore::PlainWorld{46.9181f, 142.7189f};
//
//    auto jsonSource = new KCore::GeoJSONLocalSource;
//    jsonSource->addSourcePart("assets/sources/points.geojson");
//    world->registerSource(jsonSource, "json");
//
//    auto terrainSource = new KCore::SRTMLocalSource;
//    terrainSource->addSourcePart("assets/sources/N45E141.hgt");
//    terrainSource->addSourcePart("assets/sources", ".hgt");
//    world->registerSource(terrainSource, "terrain");
//
//    auto imageSource = new KCore::RemoteSource("http://tile.openstreetmap.org/{z}/{x}/{y}.png");
//    world->registerSource(imageSource, "base");
//
//    KCore::MapCore core;
//    core.setWorldAdapter(world);

//    auto *world = new KCore::TerrainedWorld{46.9181f, 142.7189f};
//
//    auto jsonSource = new KCore::GeoJSONLocalSource;
//    jsonSource->addSourcePart("assets/sources/points.geojson");
//    world->registerSource(jsonSource, "json");
//
//    auto terrainSource = new KCore::SRTMLocalSource;
//    terrainSource->addSourcePart("assets/sources", ".hgt");
//    world->registerSource(terrainSource, "terrain");
//
//    auto imageSource = new KCore::RemoteSource("http://tile.openstreetmap.org/{z}/{x}/{y}.png");
//    world->registerSource(imageSource, "base");
//
//    KCore::MapCore core;
//    core.setWorldAdapter(world);

    auto MapCore_ptr = KCore::CreateMapCore();
    auto World_ptr = KCore::CreateTerrainedWorld(46.9181f, 142.7189f);

    auto TerrainSource_ptr = KCore::CreateSRTMLocalSource();
    SRTMAddFileGlob(TerrainSource_ptr, "../build/assets/sources", ".hgt");

    auto ImageSource_ptr = KCore::CreateRemoteSource("http://tile.openstreetmap.org/{z}/{x}/{y}.png");

    KCore::TerrainedWorldRegisterSource(World_ptr, TerrainSource_ptr, "terrain");
    KCore::TerrainedWorldRegisterSource(World_ptr, ImageSource_ptr, "base");

    auto jsonSource = new KCore::GeoJSONLocalSource;
    jsonSource->addSourcePart("assets/sources/points.geojson");
    World_ptr->registerSource(jsonSource, "json");

    World_ptr->commitWorldSetup();

    KCore::SetWorldAdapter(MapCore_ptr, World_ptr);

    auto start = std::chrono::system_clock::now();
    for (auto i = 0; i < iterations; i++) {
        cameraOpenGlSpacePosition.x += 10;
        MapCore_ptr->update(cameraProjectionMatrix, cameraViewMatrix, cameraOpenGlSpacePosition);
        auto* a = GetAsyncEventsVector(MapCore_ptr);

        int z = 0;
        auto* b = EjectAsyncEventsFromVector(a, z);

        ReleaseEventsVector(a);
    }
    auto elapsed = std::chrono::system_clock::now() - start;

    std::cout << (elapsed / std::chrono::microseconds(1)) / iterations
              << " microseconds per iterations" << std::endl;

    return 0;
}