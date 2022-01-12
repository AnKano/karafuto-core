//
// Created by Anton Shubin on 1/18/2021.
//

#include "scene/map_core.hpp"
#include "scene/utils/grid_mesh.hpp"

#ifdef __EMSCRIPTEN__

#include <emscripten/bind.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(karafuto) {
    register_vector<kcore::data_tile>("kcore_tile_vector");
    register_vector<glm::vec3>("glm_vec3_vector");
    register_vector<glm::vec2>("glm_vec2_vector");
    register_vector<uint32_t>("unsigned_int_vector");
    register_vector<float>("float_vector");
    register_vector<std::string>("string_vector");

    value_object<glm::vec2>("vec2")
            .field("x", &glm::vec2::x)
            .field("y", &glm::vec2::y);

    function("lat_lon_to_point", &kcore::geo_converter::lat_lon_to_point);
    function("point_to_lat_lon", &kcore::geo_converter::point_to_lat_lon);

    class_<kcore::grid_mesh>("mesh")
            .constructor<float, float, int>()
            .constructor<float, float, int, int>()

            .function("emscripten_get_indices_ptr", &kcore::grid_mesh::emscripten_get_indices_ptr)
            .function("emscripten_get_uvs_ptr", &kcore::grid_mesh::emscripten_get_uvs_ptr)
            .function("emscripten_get_normals_ptr", &kcore::grid_mesh::emscripten_get_normals_ptr)
            .function("emscripten_get_vertices_ptr", &kcore::grid_mesh::emscripten_get_vertices_ptr)
            .function("emscripten_get_constant_ptr", &kcore::grid_mesh::emscripten_get_constant_ptr)

            .function("emscripten_get_indices_count", &kcore::grid_mesh::emscripten_get_indices_count)
            .function("emscripten_get_uvs_count", &kcore::grid_mesh::emscripten_get_uvs_count)
            .function("emscripten_get_normals_count", &kcore::grid_mesh::emscripten_get_normals_count)
            .function("emscripten_get_vertices_count", &kcore::grid_mesh::emscripten_get_vertices_count)
            .function("emscripten_get_constant_count", &kcore::grid_mesh::emscripten_get_constant_count);

    class_<kcore::data_tile>("data_tile")
            .property("quadcode", &kcore::data_tile::get_quadcode)
            .property("side_length", &kcore::data_tile::get_side_length);

    class_<kcore::map_core>("map_core")
            .constructor<float, float>()
            .function("update", select_overload<void(intptr_t, intptr_t, intptr_t)>(&kcore::map_core::update))
            .function("get_tiles", &kcore::map_core::get_tiles)
            .function("emscripten_get_tiles", &kcore::map_core::emscripten_get_tiles)
            .function("emscripten_get_meta_tiles", &kcore::map_core::emscripten_get_meta_tiles);
}

#else

#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"

#include <iostream>
#include <chrono>

int main() {
    const uint16_t viewportWidth{ 3000 };
    const uint16_t viewportHeight{ 1800 };

    const float aspectRatio{ viewportWidth / viewportHeight };

    // create camera that reproduce equivalent point of view and matrix

    glm::mat4 cameraProjectionMatrix = glm::perspective(
        glm::radians(60.0f), aspectRatio,
        100.0f, 2500000.0f
    );

    glm::vec3 cameraOpenGlSpacePosition{ 1000.0f, 10000.0f, 10000.0f };
    glm::vec3 cameraOpenGlSpaceTarget{ 0.0f, 0.0f, 0.0f };
    glm::vec3 cameraOpenGlSpaceUp{ 0.0f, 1.0f, 0.0f };

    glm::mat4 cameraViewMatrix = glm::lookAt(
        cameraOpenGlSpacePosition,
        cameraOpenGlSpaceTarget,
        cameraOpenGlSpaceUp
    );

    // 46.9181f, 142.7189f is latitude and longitude of 
    // the surroundings of the city of Yuzhno-Sakhalinsk 

    kcore::map_core core{ 46.9181f, 142.7189f };
    auto start = std::chrono::system_clock::now();
    for (uint16_t i = 0; i < 1000; i++)
        core.update(cameraProjectionMatrix, cameraViewMatrix, cameraOpenGlSpacePosition);
    auto stop = std::chrono::system_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
    std::cout << std::chrono::duration<double>(elapsed).count() / 1000 << " seconds" << std::endl;

    return 0;
}

#endif