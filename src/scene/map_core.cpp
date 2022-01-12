//
// Created by ash on 25.09.2021.
//

#include "map_core.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace kcore {
    map_core::map_core(float latitude, float longitude) {
//        glm::vec2 origin_lat_lon{46.9181, 142.7189};

        glm::vec2 origin_lat_lon{latitude, longitude};
        glm::vec2 origin_point{kcore::geo_converter::lat_lon_to_point(origin_lat_lon)};

        m_world = std::make_shared<data_world>(origin_lat_lon, origin_point);
        m_world->update_frustum(this->m_camera_projection_matrix, this->m_camera_view_matrix);
        m_world->set_position(this->m_camera_position);
    }

    void map_core::update(const float *camera_projection_matrix_raw,
                          const float *camera_view_matrix_raw,
                          const float *camera_position_data) {
        this->m_camera_view_matrix = glm::make_mat4x4(camera_view_matrix_raw);
        this->m_camera_projection_matrix = glm::make_mat4x4(camera_projection_matrix_raw);
        this->m_camera_position = glm::make_vec3(camera_position_data);

        m_world->update_frustum(this->m_camera_projection_matrix, this->m_camera_view_matrix);
        m_world->set_position(this->m_camera_position);
        m_world->calculate_tiles();
        m_world->calculate_additional_tiles();
    }

    void map_core::update(const glm::mat4 &camera_projection_matrix,
                          const glm::mat4 &camera_view_matrix,
                          const glm::vec3 &camera_position) {
        this->m_camera_view_matrix = camera_view_matrix;
        this->m_camera_projection_matrix = camera_projection_matrix;
        this->m_camera_position = camera_position;

        m_world->update_frustum(this->m_camera_projection_matrix, this->m_camera_view_matrix);
        m_world->set_position(this->m_camera_position);
        m_world->calculate_tiles();
        m_world->calculate_additional_tiles();
    }

    const std::vector<std::shared_ptr<data_tile>> &map_core::get_tiles() {
        return m_world->get_tiles();
    }

    const std::vector<data_tile> &map_core::emscripten_get_tiles() {
        m_tiles.clear();

        for (const auto &item : get_tiles())
            m_tiles.push_back(*item);

        return std::move(m_tiles);
    }

    const std::vector<std::shared_ptr<data_tile>> &map_core::get_height_tiles() {
        return m_world->get_height_tiles();
    }

    const std::vector<data_tile> &map_core::emscripten_get_meta_tiles() {
        m_meta_tiles.clear();

        for (const auto &item : get_height_tiles())
            m_meta_tiles.push_back(*item);

        return std::move(m_meta_tiles);
    }
}