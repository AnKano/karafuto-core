//
// Created by ash on 01.10.2021.
//

#ifndef RAYLIB_RENDERER_GRID_MESH_HPP
#define RAYLIB_RENDERER_GRID_MESH_HPP

#include "glm/glm.hpp"

#include <vector>

namespace kcore {
    class grid_mesh {
    private:
        std::vector<uint8_t> m_constant;
        std::vector<glm::vec3> m_vertices;
        std::vector<glm::vec3> m_normals;
        std::vector<glm::vec2> m_uvs;
        std::vector<uint32_t> m_indices;

        uint32_t m_vertex_count{0};
        uint32_t m_triangles_count{0};

    public:
        grid_mesh(float width, float length, int segments_x, int segments_y);

        grid_mesh(float width, float length, int segments);

        ~grid_mesh() = default;

        intptr_t emscripten_get_constant_ptr();

        uint32_t emscripten_get_constant_count();

        intptr_t emscripten_get_indices_ptr();

        uint32_t emscripten_get_indices_count();

        intptr_t emscripten_get_uvs_ptr();

        uint32_t emscripten_get_uvs_count();

        intptr_t emscripten_get_normals_ptr();

        uint32_t emscripten_get_normals_count();

        intptr_t emscripten_get_vertices_ptr();

        uint32_t emscripten_get_vertices_count();

    private:
        void create_mesh(float width, float length, int segments_x, int segments_y);

        void create_general_surface(float width, float length, int segments_x, int segments_y);

        void create_border_surface_x(float width, int segments_x, float constraint,
                                     float uv_constant, float uv_interpolated_constant);

        void create_border_surface_y(float length, int segments_y, float constraint,
                                     float uv_constant, float uv_interpolated_constant);
    };
}

#endif //RAYLIB_RENDERER_GRID_MESH_HPP
