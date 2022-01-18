#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/intersect.hpp"
#include "glm/gtx/vector_angle.hpp"

#include <array>
#include <iostream>

namespace kcore {
    class frustum_culling {
    private:
        float nx_x{}, nx_y{}, nx_z{}, nx_w{};
        float px_x{}, px_y{}, px_z{}, px_w{};
        float ny_x{}, ny_y{}, ny_z{}, ny_w{};
        float py_x{}, py_y{}, py_z{}, py_w{};
        float nz_x{}, nz_y{}, nz_z{}, nz_w{};
        float pz_x{}, pz_y{}, pz_z{}, pz_w{};

        glm::mat4 project_view_matrix{};
    public:
        frustum_culling() = default;

        void update_frustum(const glm::mat4 &projection_matrix, const glm::mat4 &view_matrix) {
            this->project_view_matrix = projection_matrix * view_matrix;

            nx_x = project_view_matrix[0][3] + project_view_matrix[0][0];
            nx_y = project_view_matrix[1][3] + project_view_matrix[1][0];
            nx_z = project_view_matrix[2][3] + project_view_matrix[2][0];
            nx_w = project_view_matrix[3][3] + project_view_matrix[3][0];

            px_x = project_view_matrix[0][3] - project_view_matrix[0][0];
            px_y = project_view_matrix[1][3] - project_view_matrix[1][0];
            px_z = project_view_matrix[2][3] - project_view_matrix[2][0];
            px_w = project_view_matrix[3][3] - project_view_matrix[3][0];

            ny_x = project_view_matrix[0][3] + project_view_matrix[0][1];
            ny_y = project_view_matrix[1][3] + project_view_matrix[1][1];
            ny_z = project_view_matrix[2][3] + project_view_matrix[2][1];
            ny_w = project_view_matrix[3][3] + project_view_matrix[3][1];

            py_x = project_view_matrix[0][3] - project_view_matrix[0][1];
            py_y = project_view_matrix[1][3] - project_view_matrix[1][1];
            py_z = project_view_matrix[2][3] - project_view_matrix[2][1];
            py_w = project_view_matrix[3][3] - project_view_matrix[3][1];

            nz_x = project_view_matrix[0][3] + project_view_matrix[0][2];
            nz_y = project_view_matrix[1][3] + project_view_matrix[1][2];
            nz_z = project_view_matrix[2][3] + project_view_matrix[2][2];
            nz_w = project_view_matrix[3][3] + project_view_matrix[3][2];

            pz_x = project_view_matrix[0][3] - project_view_matrix[0][2];
            pz_y = project_view_matrix[1][3] - project_view_matrix[1][2];
            pz_z = project_view_matrix[2][3] - project_view_matrix[2][2];
            pz_w = project_view_matrix[3][3] - project_view_matrix[3][2];
        }

        [[nodiscard]]
        bool test_box(float min_x, float min_y, float min_z, float max_x, float max_y, float max_z) const {
            return nx_x * (nx_x < 0 ? min_x : max_x) +
                   nx_y * (nx_y < 0 ? min_y : max_y) +
                   nx_z * (nx_z < 0 ? min_z : max_z) >= -nx_w &&
                   px_x * (px_x < 0 ? min_x : max_x) +
                   px_y * (px_y < 0 ? min_y : max_y) +
                   px_z * (px_z < 0 ? min_z : max_z) >= -px_w &&
                   ny_x * (ny_x < 0 ? min_x : max_x) +
                   ny_y * (ny_y < 0 ? min_y : max_y) +
                   ny_z * (ny_z < 0 ? min_z : max_z) >= -ny_w &&
                   py_x * (py_x < 0 ? min_x : max_x) +
                   py_y * (py_y < 0 ? min_y : max_y) +
                   py_z * (py_z < 0 ? min_z : max_z) >= -py_w &&
                   nz_x * (nz_x < 0 ? min_x : max_x) +
                   nz_y * (nz_y < 0 ? min_y : max_y) +
                   nz_z * (nz_z < 0 ? min_z : max_z) >= -nz_w &&
                   pz_x * (pz_x < 0 ? min_x : max_x) +
                   pz_y * (pz_y < 0 ? min_y : max_y) +
                   pz_z * (pz_z < 0 ? min_z : max_z) >= -pz_w;
        }
    };
}
