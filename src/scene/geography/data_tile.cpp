//
// Created by Ash on 2/6/2021.
//

#include "data_tile.hpp"

#include <iostream>

namespace kcore {
    data_tile::data_tile(const std::string &quadcode) {
        if (quadcode.empty())
            throw std::invalid_argument("empty quadcode");
        
        set_quadcode(quadcode);
        cardinals = eject_cardinals();
    }

    void data_tile::set_quadcode(const std::string& quadcode) {
        data_tile::quadcode = quadcode;
    }

    void data_tile::set_parent(const data_tile *parent) {
        data_tile::parent = parent;
        set_type((parent == nullptr) ? data_tile_type::root : data_tile_type::leaf);
    }

    void data_tile::set_tilecode(const glm::ivec3 &tilecode) {
        data_tile::payload.tile_code.x = static_cast<int32_t>(tilecode.x);
        data_tile::payload.tile_code.y = static_cast<int32_t>(tilecode.y);
        data_tile::payload.tile_code.z = static_cast<int32_t>(tilecode.z);
        data_tile::tilecode = tilecode;
    }

    void data_tile::set_bounds_lat_lon(const glm::vec4 &bounds_lat_lon) {
        data_tile::bounds_lat_lon = bounds_lat_lon;
    }

    void data_tile::set_bounds_world(const glm::vec4 &bounds_world) {
        data_tile::bounds_world = bounds_world;
    }

    void data_tile::set_center(const glm::vec2 &center) {
        data_tile::payload.center[0] = center.x;
        data_tile::payload.center[1] = center.y;
        data_tile::center = center;
    }

    void data_tile::set_center_lat_lon(const glm::vec2 &center_lat_lon) {
        data_tile::center_lat_lon = center_lat_lon;
    }

    void data_tile::set_side_length(float side_length) {
        data_tile::payload.side_length = side_length;
        data_tile::side_length = side_length;
    }

    void data_tile::set_type(data_tile_type type) {
        data_tile::payload.type = type;
        data_tile::type = type;
    }

    void data_tile::set_visibility(data_tile_visibility visibility) {
        data_tile::payload.visibility = visibility;
        data_tile::visibility = visibility;
    }

    const std::string &data_tile::get_quadcode() const {
        return quadcode;
    }

    const glm::ivec3 &data_tile::get_tilecode() const {
        return tilecode;
    }

    const glm::vec4 &data_tile::get_bounds_lat_lon() const {
        return bounds_lat_lon;
    }

    const glm::vec4 &data_tile::get_bounds_world() const {
        return bounds_world;
    }

    const glm::vec2 &data_tile::get_center() const {
        return center;
    }

    const glm::vec2 &data_tile::get_center_lat_lon() const {
        return center_lat_lon;
    }

    float data_tile::get_side_length() const {
        return side_length;
    }

    data_tile_type data_tile::get_type() const {
        return type;
    }

    data_tile_visibility data_tile::get_visibility() const {
        return visibility;
    }

    enum data_tile_cardinals data_tile::eject_cardinals() {
        switch (quadcode.back()) {
            case '0': return north_west;
            case '1': return north_east;
            case '2': return south_west;
            case '3': return south_east;
            default:
                throw std::runtime_error("Unexpected tile cardinals");
        }
    }
}