//
// Created by ash on 01.10.2021.
//

#include <string>
#include "grid_mesh.hpp"

namespace kcore {
    grid_mesh::grid_mesh(float width, float length, int segments) {
        create_mesh(width, length, segments, segments);
    }

    grid_mesh::grid_mesh(float width, float length, int segments_x, int segments_y) {
        create_mesh(width, length, segments_x, segments_y);
    }

    void grid_mesh::create_mesh(float width, float length, int segments_x, int segments_y) {
        create_general_surface(width, length, segments_x, segments_y);

        // north and south borders
        create_border_surface_x(width, segments_x, -0.5f, 0.0f, -0.1f);
        create_border_surface_x(width, segments_x, 0.5f, 1.0f, 1.1f);

        // east and west borders
        create_border_surface_y(length, segments_y, -0.5f, 0.0f, -0.1f);
        create_border_surface_y(length, segments_y, 0.5f, 1.0f, 1.1f);

        m_vertex_count = m_vertices.size();
    }

    void grid_mesh::create_general_surface(float width, float length, int segments_x, int segments_y) {
        segments_y *= 2;
        segments_x *= 2;

        const float width_half = width / 2.0f;
        const float height_half = length / 2.0f;

        const int grid_x = segments_x + 1;
        const int grid_y = segments_y + 1;

        const float segment_width = width / ((float) segments_x);
        const float segment_height = length / ((float) segments_y);

        for (int iy = 0; iy < grid_y; iy++) {
            for (int ix = 0; ix < grid_x; ix++) {
                float x = ((float) ix * segment_width) - width_half;
                float y = ((float) iy * segment_height) - height_half;

                m_vertices.emplace_back(x, 0.0f, y);
                m_normals.emplace_back(0.0f, 1.0f, 0.0f);
                m_constant.emplace_back(false);

                float uvs_ix = (float) ix / ((float) segments_x);
                float uvs_iy = (float) iy / ((float) segments_y);

                m_uvs.emplace_back(uvs_ix, 1.0f - uvs_iy);
            }
        }

        for (int iy = 0; iy < segments_y; iy += 2) {
            for (int ix = 0; ix < segments_x; ix += 2) {
                const uint32_t a = ix + grid_x * iy;
                const uint32_t b = ix + grid_x * (iy + 2);

                // index of center of each segment
                const uint32_t x = (ix + 1) + grid_x * (iy + 1);

                const uint32_t c = (ix + 2) + grid_x * (iy + 2);
                const uint32_t d = (ix + 2) + grid_x * iy;

                m_indices.insert(m_indices.end(), {a, b, x});
                m_indices.insert(m_indices.end(), {b, c, x});
                m_indices.insert(m_indices.end(), {c, d, x});
                m_indices.insert(m_indices.end(), {d, a, x});
            }
        }

        m_triangles_count = (segments_x * segments_y) * 2;
    }

    void grid_mesh::create_border_surface_x(float width, int segments_x, float constraint,
                                            float uv_constant, float uv_interpolated_constant) {
        uint32_t vertices_count = m_vertices.size();

        int segments_y = 1;

        const float width_half = width / 2.0f;
        const int grid_x = segments_x + 1;
        const float segment_width = width / ((float) segments_x);

        for (int iy = 0; iy < 2; iy++) {
            for (int ix = 0; ix < grid_x; ix++) {
                float x = ((float) ix * segment_width) - width_half;
                float z = (iy == 0) ? 0.0f : -1.0f;

                float uvs_ix = (float) ix / ((float) segments_x);
                float uvs_iy = (iy == 0) ? uv_constant : uv_interpolated_constant;

                m_vertices.emplace_back(x, z, constraint);
                m_normals.emplace_back(0.0f, 1.0f, 0.0f);
                m_uvs.emplace_back(uvs_ix, 1.0f - uvs_iy);
                m_constant.emplace_back(iy != 0);
            }
        }

        for (int iy = 0; iy < segments_y; iy++) {
            for (int ix = 0; ix < segments_x; ix++) {
                uint32_t a = ix + grid_x * iy;
                uint32_t b = ix + grid_x * (iy + 1);
                uint32_t c = (ix + 1) + grid_x * (iy + 1);
                uint32_t d = (ix + 1) + grid_x * iy;

                a += vertices_count;
                b += vertices_count;
                c += vertices_count;
                d += vertices_count;

                m_indices.insert(m_indices.end(), {a, b, d});
                m_indices.insert(m_indices.end(), {b, c, d});
            }
        }

        m_triangles_count += (segments_x * segments_y) * 2;
    }

    void grid_mesh::create_border_surface_y(float length, int segments_y, float constraint,
                                            float uv_constant, float uv_interpolated_constant) {
        uint32_t vertices_count = m_vertices.size();

        int segments_x = 1;

        const float height_half = length / 2.0f;
        const int grid_y = segments_y + 1;
        const float segment_height = length / ((float) segments_y);

        for (int iy = 0; iy < grid_y; iy++) {
            for (int ix = 0; ix < 2; ix++) {
                float y = ((float) iy * segment_height) - height_half;
                float z = (ix == 0) ? 0.0f : -1.0f;

                float uvs_ix = (iy == 0) ? uv_constant : uv_interpolated_constant;
                float uvs_iy = (float) iy / ((float) segments_y);

                m_vertices.emplace_back(constraint, z, y);
                m_normals.emplace_back(0.0f, 1.0f, 0.0f);
                m_uvs.emplace_back(uvs_ix, 1.0f - uvs_iy);
                m_constant.emplace_back(ix != 0);
            }
        }

        for (int iy = 0; iy < segments_y; iy++) {
            for (int ix = 0; ix < segments_x; ix++) {
                uint32_t a = ix + 2 * iy;
                uint32_t b = ix + 2 * (iy + 1);
                uint32_t c = (ix + 1) + 2 * (iy + 1);
                uint32_t d = (ix + 1) + 2 * iy;

                a += vertices_count;
                b += vertices_count;
                c += vertices_count;
                d += vertices_count;

                m_indices.insert(m_indices.end(), {a, b, d});
                m_indices.insert(m_indices.end(), {b, c, d});
            }
        }

        m_triangles_count += (segments_x * segments_y) * 2;
    }

    intptr_t grid_mesh::emscripten_get_indices_ptr() {
        return reinterpret_cast<intptr_t>(m_indices.data());
    }

    uint32_t grid_mesh::emscripten_get_indices_count() {
        return m_indices.size();
    }

    intptr_t grid_mesh::emscripten_get_uvs_ptr() {
        return reinterpret_cast<intptr_t>(m_uvs.data());
    }

    uint32_t grid_mesh::emscripten_get_uvs_count() {
        return m_uvs.size() * 2;
    }

    intptr_t grid_mesh::emscripten_get_normals_ptr() {
        return reinterpret_cast<intptr_t>(m_normals.data());
    }

    uint32_t grid_mesh::emscripten_get_normals_count() {
        return m_normals.size() * 3;
    }

    intptr_t grid_mesh::emscripten_get_vertices_ptr() {
        return reinterpret_cast<intptr_t>(m_vertices.data());
    }

    uint32_t grid_mesh::emscripten_get_vertices_count() {
        return m_vertices.size() * 3;
    }

    intptr_t grid_mesh::emscripten_get_constant_ptr() {
        return reinterpret_cast<intptr_t>(m_constant.data());
    }

    uint32_t grid_mesh::emscripten_get_constant_count() {
        return m_constant.size();
    }
}