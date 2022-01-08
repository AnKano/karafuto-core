// Created by ash on 25.09.2021.
//

#ifndef A_SIMPLE_TRIANGLE_WINDOWS_MAPCORE_HPP
#define A_SIMPLE_TRIANGLE_WINDOWS_MAPCORE_HPP

#include <vector>
#include "glm/glm.hpp"
#include "geo/data_tile.hpp"
#include "geo/data_layer.hpp"

namespace kcore {
    class map_core {
    public:
        map_core();

        map_core(const float *camera_projection_matrix_raw,
                 const float *camera_view_matrix_raw,
                 const float *camera_position_data);

        map_core(const glm::mat4 &camera_projection_matrix,
                 const glm::mat4 &camera_view_matrix,
                 const glm::vec3 &camera_position);

        void update(const float *camera_projection_matrix_raw,
                    const float *camera_view_matrix_raw,
                    const float *camera_position_data);

        void update(const glm::mat4 &camera_projection_matrix,
                    const glm::mat4 &camera_view_matrix,
                    const glm::vec3 &camera_position);

        map_core(intptr_t camera_projection_matrix_addr,
                 intptr_t camera_view_matrix_addr,
                 intptr_t camera_position_addr) :
                map_core(reinterpret_cast<float *>(camera_projection_matrix_addr),
                         reinterpret_cast<float *>(camera_view_matrix_addr),
                         reinterpret_cast<float *>(camera_position_addr)) {}

        void update(intptr_t camera_projection_matrix_addr,
                    intptr_t camera_view_matrix_addr,
                    intptr_t camera_position_addr) {
            return update(reinterpret_cast<float *>(camera_projection_matrix_addr),
                          reinterpret_cast<float *>(camera_view_matrix_addr),
                          reinterpret_cast<float *>(camera_position_addr));
        }

        const std::vector<data_tile> &emscripten_get_tiles();

        const std::vector<std::shared_ptr<data_tile>> &get_tiles();

        const std::vector<std::shared_ptr<data_tile>> &get_height_tiles();

    private:
        glm::mat4 m_camera_view_matrix{}, m_camera_projection_matrix{};
        glm::vec3 m_camera_position{};
        std::vector<data_layer> m_layers;

        std::vector<data_tile> tiles;
    };
}

#endif //A_SIMPLE_TRIANGLE_WINDOWS_MAPCORE_HPP
