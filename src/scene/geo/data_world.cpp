//
// Created by Ash on 2/6/2021.
//

#include "data_world.hpp"

#include <algorithm>

namespace kcore {
    glm::vec2 data_world::lat_lon_to_world_point(const glm::vec2 &lat_lon) {
        auto projected_point = geo_converter::lat_lon_to_point(lat_lon);
        return projected_point - m_origin_lat_lon;
    }

    glm::vec2 data_world::point_to_world_lat_lon(const glm::vec2 &point) {
        auto projected_point = point + m_origin_lat_lon;
        return geo_converter::point_to_lat_lon(projected_point);
    }

    data_world::data_world(const glm::vec2 &origin_lat_lon, const glm::vec2 &origin_point) :
            m_origin_lat_lon(origin_point) {}

    void data_world::update_frustum(const glm::mat4 &projection_matrix, const glm::mat4 &view_matrix) {
        m_culling.update_frustum(projection_matrix, view_matrix);
    }

    std::shared_ptr<data_tile> data_world::create_tile(const std::string &quadcode, const data_tile *parent) {
        std::shared_ptr<data_tile> tile = std::make_shared<data_tile>(quadcode);
        tile->set_parent(parent);

        tile->set_tilecode(geo_converter::quadcode_to_tilecode(quadcode));

        {
            const glm::vec3 &tilecode = tile->get_tilecode();
            auto w = geo_converter::tile_to_lon(tilecode[0], tilecode[2]);
            auto s = geo_converter::tile_to_lat(tilecode[1] + 1, tilecode[2]);
            auto e = geo_converter::tile_to_lon(tilecode[0] + 1, tilecode[2]);
            auto n = geo_converter::tile_to_lat(tilecode[1], tilecode[2]);

            tile->set_bounds_lat_lon({w, s, e, n});
        }

        {
            const glm::vec4 &bounds_lat_lon = tile->get_bounds_lat_lon();
            auto sw = lat_lon_to_world_point({bounds_lat_lon[1], bounds_lat_lon[0]});
            auto ne = lat_lon_to_world_point({bounds_lat_lon[3], bounds_lat_lon[2]});
            tile->set_bounds_world({sw.x, sw.y, ne.x, ne.y});
        }

        {
            const glm::vec4 &bounds_world = tile->get_bounds_world();
            auto x = bounds_world[0] + (bounds_world[2] - bounds_world[0]) / 2;
            auto y = bounds_world[1] + (bounds_world[3] - bounds_world[1]) / 2;

            tile->set_center({x, y});

            glm::vec3 first_point = glm::vec3(bounds_world[0], 0, bounds_world[3]);
            glm::vec3 second_point = glm::vec3(bounds_world[0], 0, bounds_world[1]);

            tile->set_side_length(glm::length(first_point - second_point));
        }

        tile->set_center_lat_lon(point_to_world_lat_lon(tile->get_center()));
        return tile;
    }

    void data_world::calculate_tiles() {
        // clear up all tiles
        m_tiles.clear();

        // create root nodes
        m_tiles.emplace_back(create_tile("0", nullptr));
        m_tiles.emplace_back(create_tile("1", nullptr));
        m_tiles.emplace_back(create_tile("2", nullptr));
        m_tiles.emplace_back(create_tile("3", nullptr));

        std::size_t count = 0;
        while (count != m_tiles.size()) {
            auto *tile = m_tiles[count].get();
            auto quadcode = tile->get_quadcode();

            if (screen_space_error(*tile, 3.0)) {
                if (tile->get_type() != data_tile_type::root)
                    tile->set_type(data_tile_type::separated);
                tile->set_visibility(data_tile_visibility::hide);

                auto tile_nw = create_tile(quadcode + "0", tile);
                auto tile_ne = create_tile(quadcode + "1", tile);
                auto tile_sw = create_tile(quadcode + "2", tile);
                auto tile_se = create_tile(quadcode + "3", tile);

                m_tiles.emplace_back(tile_nw);
                m_tiles.emplace_back(tile_ne);
                m_tiles.emplace_back(tile_sw);
                m_tiles.emplace_back(tile_se);

                tile->childs.quad[north_west] = tile_nw;
                tile->childs.quad[north_east] = tile_ne;
                tile->childs.quad[south_west] = tile_sw;
                tile->childs.quad[south_east] = tile_se;

                tile->childs.sides[north][0] = tile_nw;
                tile->childs.sides[north][1] = tile_ne;

                tile->childs.sides[south][0] = tile_sw;
                tile->childs.sides[south][1] = tile_se;

                tile->childs.sides[west][0] = tile_nw;
                tile->childs.sides[west][1] = tile_sw;

                tile->childs.sides[east][0] = tile_ne;
                tile->childs.sides[east][1] = tile_se;
            }

            count++;
        }
    }

    void data_world::calculate_additional_tiles() {
        m_meta_tiles.clear();

        auto depth = calculate_maximal_depth();

        // create height nodes
        m_meta_tiles.emplace_back(create_tile("0", nullptr));
        m_meta_tiles.emplace_back(create_tile("1", nullptr));
        m_meta_tiles.emplace_back(create_tile("2", nullptr));
        m_meta_tiles.emplace_back(create_tile("3", nullptr));

        std::size_t count = 0;
        while (count != m_meta_tiles.size()) {
            auto *tile = m_meta_tiles[count].get();
            auto quadcode = tile->get_quadcode();

            if (targeted_screen_space_error(*tile, depth)) {
                if (tile->get_type() != data_tile_type::root)
                    tile->set_type(data_tile_type::separated);
                tile->set_visibility(data_tile_visibility::hide);

                auto tile_nw = create_tile(quadcode + "0", tile);
                auto tile_ne = create_tile(quadcode + "1", tile);
                auto tile_sw = create_tile(quadcode + "2", tile);
                auto tile_se = create_tile(quadcode + "3", tile);

                m_meta_tiles.emplace_back(tile_nw);
                m_meta_tiles.emplace_back(tile_ne);
                m_meta_tiles.emplace_back(tile_sw);
                m_meta_tiles.emplace_back(tile_se);

                tile->childs.quad[north_west] = tile_nw;
                tile->childs.quad[north_east] = tile_ne;
                tile->childs.quad[south_west] = tile_sw;
                tile->childs.quad[south_east] = tile_se;

                tile->childs.sides[north][0] = tile_nw;
                tile->childs.sides[north][1] = tile_ne;

                tile->childs.sides[south][0] = tile_sw;
                tile->childs.sides[south][1] = tile_se;

                tile->childs.sides[west][0] = tile_nw;
                tile->childs.sides[west][1] = tile_sw;

                tile->childs.sides[east][0] = tile_ne;
                tile->childs.sides[east][1] = tile_se;
            }

            count++;
        }
    }

    uint8_t data_world::calculate_maximal_depth() {
        uint8_t depth{0};
        for (const auto &item: m_tiles)
            if (item->get_tilecode().z > depth)
                depth = item->get_tilecode().z;
        return depth;
    }

    bool data_world::targeted_screen_space_error(data_tile &tile, const uint8_t &depth) {
        const auto &quadcode = tile.get_quadcode();

        if (!check_tile_in_frustum(tile)) {
            // if it's not in frustum just hide it without separation
            tile.set_visibility(data_tile_visibility::hide);
            return false;
        }

        return (depth - 3) >= tile.get_tilecode().z;
    }

    bool data_world::screen_space_error(data_tile &tile, float quality = 3.0f) {
        const auto &quadcode = tile.get_quadcode();

        if (!check_tile_in_frustum(tile)) {
            // if it's not in frustum just hide it without separation
            tile.set_visibility(data_tile_visibility::hide);
            return false;
        }

        auto center = tile.get_center();
        auto distance = glm::length(glm::vec3(center.x, 0, center.y) - m_origin_position);
        auto error = quality * tile.get_side_length() / distance;

        return error > 1.0f;
    }

    bool data_world::check_tile_in_frustum(const data_tile &tile) {
        auto pos = tile.get_center();
        auto scale = tile.get_side_length();

        float min_x = (float) pos.x - scale, max_x = (float) pos.x + scale;
        float min_z = (float) pos.y - scale, max_z = (float) pos.y + scale;
        float min_y = 0.0f, max_y = 0.0f;

        auto result = m_culling.test_box(min_x, min_y, min_z, max_x, max_y, max_z);
        return result;
    }

    void data_world::set_position(const glm::vec3 &position) {
        m_origin_position = position;
    }

    const std::vector<std::shared_ptr<data_tile>> &data_world::get_tiles() {
        return m_tiles;
    }

    const std::vector<std::shared_ptr<data_tile>> &data_world::get_height_tiles() {
        return m_meta_tiles;
    }
}