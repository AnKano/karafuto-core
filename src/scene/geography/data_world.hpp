#pragma once

#include <list>
#include <map>
#include <optional>

#include "glm/glm.hpp"

#include "data_tile.hpp"
#include "../misc/frustum_culling.hpp"

namespace kcore {
    class data_world {
        kcore::frustum_culling m_culling{};

        std::list<data_tile> m_tiles{};
        std::list<data_tile> m_meta_tiles{};

        std::map<std::string, data_tile*> m_tiles_tree;

        glm::vec2 m_origin_wgs84_position{};
        glm::vec3 m_origin_ingraphics_position{};

        bool screen_space_error(data_tile &tile, float quality);

        bool targeted_screen_space_error(data_tile& tile, const uint8_t& depth);

        bool check_tile_in_frustum(const data_tile &tile);

        data_tile create_tile(const std::string &quadcode, const data_tile *parent);

        data_tile find_or_create_tile(const std::string& quadcode, const data_tile* parent);

        uint8_t calculate_maximal_depth();

    public:
        data_world(const glm::vec2 &origin_lat_lon, const glm::vec2 &origin_point);

        glm::vec2 lat_lon_to_world_point(const glm::vec2 &lat_lon);

        glm::vec2 point_to_world_lat_lon(const glm::vec2 &point);

        void update_frustum(const glm::mat4 &projection_matrix,
                            const glm::mat4 &view_matrix);

        void set_position(const glm::vec3 &position);

        const std::list<data_tile> &get_tiles();

        const std::list<data_tile> &get_meta_tiles();

        void calculate_tiles();

        void calculate_meta_tiles();
    };
}

