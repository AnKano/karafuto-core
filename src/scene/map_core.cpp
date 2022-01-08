//
// Created by ash on 25.09.2021.
//

#include "map_core.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace kcore {
    map_core::map_core() {
        glm::vec2 origin_lat_lon{46.9181, 142.7189};
        glm::vec2 origin_point{kcore::geo_converter::lat_lon_to_point(origin_lat_lon)};
        m_layers.emplace_back(origin_lat_lon, origin_point);

        m_layers[0].update_frustum(this->m_camera_projection_matrix, this->m_camera_view_matrix);
        m_layers[0].set_position(this->m_camera_position);
    }

    map_core::map_core(const glm::mat4 &camera_projection_matrix,
                       const glm::mat4 &camera_view_matrix,
                       const glm::vec3 &camera_position) :
            m_camera_view_matrix(camera_projection_matrix),
            m_camera_projection_matrix(camera_view_matrix),
            m_camera_position(camera_position) {
        glm::vec2 origin_lat_lon{46.9181, 142.7189};
        glm::vec2 origin_point{kcore::geo_converter::lat_lon_to_point(origin_lat_lon)};

        m_layers.emplace_back(origin_lat_lon, origin_point);

        m_layers[0].update_frustum(this->m_camera_projection_matrix, this->m_camera_view_matrix);
        m_layers[0].set_position(this->m_camera_position);

        m_layers[0].calculate_tiles();
        m_layers[0].calculate_additional_tiles();
    }

    map_core::map_core(const float *camera_projection_matrix_raw,
                       const float *camera_view_matrix_raw,
                       const float *camera_position_data) :
            m_camera_view_matrix(glm::make_mat4x4(camera_view_matrix_raw)),
            m_camera_projection_matrix(glm::make_mat4x4(camera_projection_matrix_raw)),
            m_camera_position(glm::make_vec3(camera_position_data)) {
        glm::vec2 origin_lat_lon{46.9181, 142.7189};
        glm::vec2 origin_point{kcore::geo_converter::lat_lon_to_point(origin_lat_lon)};

        m_layers.emplace_back(origin_lat_lon, origin_point);

        m_layers[0].update_frustum(this->m_camera_projection_matrix, this->m_camera_view_matrix);
        m_layers[0].set_position(this->m_camera_position);

        m_layers[0].calculate_tiles();
        m_layers[0].calculate_additional_tiles();
    }

    void map_core::update(const float *camera_projection_matrix_raw,
                          const float *camera_view_matrix_raw,
                          const float *camera_position_data) {
        this->m_camera_view_matrix = glm::make_mat4x4(camera_view_matrix_raw);
        this->m_camera_projection_matrix = glm::make_mat4x4(camera_projection_matrix_raw);
        this->m_camera_position = glm::make_vec3(camera_position_data);

        m_layers[0].update_frustum(this->m_camera_projection_matrix, this->m_camera_view_matrix);
        m_layers[0].set_position(this->m_camera_position);

        m_layers[0].calculate_tiles();
        m_layers[0].calculate_additional_tiles();
    }

    void map_core::update(const glm::mat4 &camera_projection_matrix,
                          const glm::mat4 &camera_view_matrix,
                          const glm::vec3 &camera_position) {
        this->m_camera_view_matrix = camera_view_matrix;
        this->m_camera_projection_matrix = camera_projection_matrix;
        this->m_camera_position = camera_position;

        m_layers[0].update_frustum(this->m_camera_projection_matrix, this->m_camera_view_matrix);
        m_layers[0].set_position(this->m_camera_position);

        m_layers[0].calculate_tiles();
        m_layers[0].calculate_additional_tiles();
    }

    const std::vector<std::shared_ptr<data_tile>> &map_core::get_tiles() {
        return m_layers[0].get_tiles();
    }

    const std::vector<data_tile> &map_core::emscripten_get_tiles() {
        tiles.clear();

        for (const auto &item : m_layers[0].get_tiles())
            tiles.push_back(*item);

        return std::move(tiles);
    }

    const std::vector<std::shared_ptr<data_tile>> &map_core::get_height_tiles() {
        return m_layers[0].get_height_tiles();
    }
}