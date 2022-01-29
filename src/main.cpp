//
// Created by Anton Shubin on 1/18/2021.
//


#include "core/MapCore.hpp"

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
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <chrono>

int main() {
    const uint16_t viewportWidth{2560};
    const uint16_t viewportHeight{1259};

    const float aspectRatio{(float) viewportWidth / viewportHeight};

    // create camera that reproduce equivalent point of view and matrix

    glm::mat4 cameraProjectionMatrix = glm::perspective(
            glm::radians(60.0f), aspectRatio,
            0.1f, 2500000.0f
    );

    glm::vec3 cameraOpenGlSpacePosition{1000.0f, 10000.0f, 10000.0f};
    glm::vec3 cameraOpenGlSpaceTarget{0.0f, 0.0f, 0.0f};
    glm::vec3 cameraOpenGlSpaceUp{0.0f, 1.0f, 0.0f};

    glm::mat4 cameraViewMatrix = glm::lookAt(
            cameraOpenGlSpacePosition,
            cameraOpenGlSpaceTarget,
            cameraOpenGlSpaceUp
    );

    const uint16_t iteration = 1000;

//     46.9181f, 142.7189f is latitude and longitude of
//     the surroundings of the city of Yuzhno-Sakhalinsk
    KCore::MapCore core{46.9181f, 142.7189f};
    auto start = std::chrono::system_clock::now();
    for (uint16_t i = 0; i < iteration; i++) {
        core.update(cameraProjectionMatrix, cameraViewMatrix, cameraOpenGlSpacePosition);
        auto val = core.getTiles();
        for (const auto &item: val) {
            auto a = std::to_string(item.mDescription.Center[0]) + "/" + std::to_string(item.mDescription.Center[1]);
        }
    }
    auto stop = std::chrono::system_clock::now();

    auto elapsed = stop - start;
    std::cout << (elapsed / std::chrono::microseconds(1)) / iteration << " microseconds per iteration" << std::endl;

//    KCore::MapCore core{46.9181f, 142.7189f};
    core.update(cameraProjectionMatrix, cameraViewMatrix, cameraOpenGlSpacePosition);

    std::this_thread::sleep_for(4s);

    core.update(cameraProjectionMatrix, cameraViewMatrix, cameraOpenGlSpacePosition);
    core.update(cameraProjectionMatrix, cameraViewMatrix, cameraOpenGlSpacePosition);

    std::this_thread::sleep_for(100s);

    return 0;
}