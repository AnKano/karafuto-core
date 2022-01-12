//
// Created by Ash on 2/6/2021.
//

#ifndef A_SIMPLE_TRIANGLE_WINDOWS_DATALAYER_HPP
#define A_SIMPLE_TRIANGLE_WINDOWS_DATALAYER_HPP

#include <vector>
#include <list>
#include <map>
#include <optional>

#include "glm/glm.hpp"
#include <unordered_map>
#include "data_tile.hpp"
#include "../utils/frustum_culling.hpp"

namespace kcore {
    class data_world {
        kcore::frustum_culling m_culling{};

        std::vector<std::shared_ptr<data_tile>> m_tiles{};
        std::vector<std::shared_ptr<data_tile>> m_meta_tiles{};

        glm::vec2 m_origin_lat_lon{};
        glm::vec3 m_origin_position{};

        bool screen_space_error(data_tile &tile, float quality);

        bool check_tile_in_frustum(const data_tile &tile);

        std::shared_ptr<data_tile> create_tile(const std::string &quadcode,
                                               const data_tile *parent);

    public:
        data_world(const glm::vec2 &origin_lat_lon,
                   const glm::vec2 &origin_point);

        glm::vec2 lat_lon_to_world_point(const glm::vec2 &lat_lon);

        glm::vec2 point_to_world_lat_lon(const glm::vec2 &point);

        void update_frustum(const glm::mat4 &projection_matrix,
                            const glm::mat4 &view_matrix);

        void calculate_tiles();

        void set_position(const glm::vec3 &position);

        const std::vector<std::shared_ptr<data_tile>> &get_tiles();

        const std::vector<std::shared_ptr<data_tile>> &get_height_tiles();

        uint8_t calculate_maximal_depth();

        void calculate_additional_tiles();

        bool targeted_screen_space_error(data_tile &tile, const uint8_t &depth);
    };
}

#endif //A_SIMPLE_TRIANGLE_WINDOWS_DATALAYER_HPP
