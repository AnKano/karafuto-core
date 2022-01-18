//
// Created by Anton Shubin on 1/18/2021.
//

#include "scene/map_core.hpp"
#include "scene/misc/grid_mesh.hpp"

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

#endif

#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"

#include <iostream>
#include <chrono>

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct TileDescription {
    std::string quadcode;
    glm::vec3 tileCode;
    glm::vec2 center;
    float sideLength;
    uint32_t type;
    uint32_t visibility;
};

int main() {
    if (!glfwInit())
        std::cout << "Not initialized!" << std::endl;

    GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
    if (!window)
    {
        // Window or OpenGL context creation failed
    }

    glfwTerminate();
    std::cout << "Terminated!" << std::endl;

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

    const uint16_t iteration = 1000;

    std::vector<TileDescription> descriptions{};

    // 46.9181f, 142.7189f is latitude and longitude of
    // the surroundings of the city of Yuzhno-Sakhalinsk

    kcore::map_core core{46.9181f, 142.7189f};
    auto start = std::chrono::system_clock::now();
    for (uint16_t i = 0; i < iteration; i++) {
        descriptions.clear();

        core.update(cameraProjectionMatrix, cameraViewMatrix, cameraOpenGlSpacePosition);

        auto tiles = core.get_tiles();
        for (const auto &item: tiles) {
            TileDescription description;
            description.quadcode = item.get_quadcode();
            description.tileCode = { item.get_tilecode().x, item.get_tilecode().y, item.get_tilecode().z };
            description.center = { item.get_center().x, item.get_center().y };
            description.sideLength = item.get_side_length();
            description.type = item.get_type();
            description.visibility = item.get_visibility();
            
            descriptions.push_back(description);
        }
    }
    auto stop = std::chrono::system_clock::now();

    auto elapsed = stop - start;
    std::cout << (elapsed / std::chrono::microseconds(1)) / iteration << " microseconds per iteration" << std::endl;

    return 0;
}