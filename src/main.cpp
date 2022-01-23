//
// Created by Anton Shubin on 1/18/2021.
//

#include "core/MapCore.hpp"
#include "core/meshes/GridMesh.hpp"

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

#include "glm/glm.hpp"

#include <iostream>
#include <chrono>

struct TileDescription {
    std::string quadcode;
    glm::vec3 tileCode;
    glm::vec2 center;
    float sideLength;
    uint32_t type;
    uint32_t visibility;
};

int main() {
    const uint16_t viewportWidth{3000};
    const uint16_t viewportHeight{1800};

    const float aspectRatio{viewportWidth / viewportHeight};

    // create camera that reproduce equivalent point of view and matrix

    glm::mat4 cameraProjectionMatrix = glm::perspective(
            glm::radians(60.0f), aspectRatio,
            100.0f, 2500000.0f
    );

    glm::vec3 cameraOpenGlSpacePosition{1000.0f, 10000.0f, 10000.0f};
    glm::vec3 cameraOpenGlSpaceTarget{0.0f, 0.0f, 0.0f};
    glm::vec3 cameraOpenGlSpaceUp{0.0f, 1.0f, 0.0f};

    glm::mat4 cameraViewMatrix = glm::lookAt(
            cameraOpenGlSpacePosition,
            cameraOpenGlSpaceTarget,
            cameraOpenGlSpaceUp
    );

    const uint16_t iteration = 100;

    std::vector<TileDescription> descriptions{};

//     46.9181f, 142.7189f is latitude and longitude of
//     the surroundings of the city of Yuzhno-Sakhalinsk
    KCore::MapCore core{46.9181f, 142.7189f};
    auto start = std::chrono::system_clock::now();
    for (uint16_t i = 0; i < iteration; i++) {
        descriptions.clear();

        core.update(cameraProjectionMatrix, cameraViewMatrix, cameraOpenGlSpacePosition);

        auto tiles = core.getTiles();
        for (const auto &item: tiles) {
            TileDescription description;
            description.quadcode = item.getQuadcode();
            description.tileCode = {item.getTilecode().x, item.getTilecode().y, item.getTilecode().z };
            description.center = {item.getCenter().x, item.getCenter().y };
            description.sideLength = item.getSideLength();
            description.type = item.getType();
            description.visibility = item.getVisibility();

            descriptions.push_back(description);
        }
    }
    auto stop = std::chrono::system_clock::now();

    auto elapsed = stop - start;
    std::cout << (elapsed / std::chrono::microseconds(1)) / iteration << " microseconds per iteration" << std::endl;

//    KCore::MapCore core{46.9181f, 142.7189f};
//    core.update(cameraProjectionMatrix, cameraViewMatrix, cameraOpenGlSpacePosition);
//
//    std::this_thread::sleep_for(10s);
//
//    core.update(cameraProjectionMatrix, cameraViewMatrix, cameraOpenGlSpacePosition);
//
//    std::this_thread::sleep_for(10s);

    return 0;
}