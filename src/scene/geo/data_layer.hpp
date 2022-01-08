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
    class data_layer {
        struct heights {
            bool created{false};

            std::vector<unsigned short> north_border;
            std::vector<unsigned short> south_border;
            std::vector<unsigned short> west_border;
            std::vector<unsigned short> east_border;

            const std::vector<unsigned short> *redirect(const data_tile_side &side) {
                if (!created) return nullptr;

                switch (side) {
                    case data_tile_side::north: return &north_border;
                    case data_tile_side::south: return &south_border;
                    case data_tile_side::west: return &west_border;
                    case data_tile_side::east: return &east_border;
                    default: throw std::runtime_error("Unexpected side");
                }
            }
        };

        int maximal_depth{INT_MIN};

        std::unordered_map<std::string, bool> load_state{};
        std::unordered_map<std::string, kcore::data_layer::heights> border_heights{};

        kcore::frustum_culling filter{};

        std::vector<std::shared_ptr<data_tile>> tiles{};
        std::vector<std::shared_ptr<data_tile>> height_tiles{};

        glm::vec2 origin_lat_lon{};
        glm::vec3 origin_position{};

        bool screen_space_error(data_tile &tile, float quality);

        bool check_tile_in_frustum(const data_tile &tile);

        std::shared_ptr<data_tile> create_tile(const std::string &quadcode,
                                               const data_tile *parent);

    public:
        data_layer(const glm::vec2 &origin_lat_lon,
                   const glm::vec2 &origin_point);

        glm::vec2 lat_lon_to_world_point(const glm::vec2 &lat_lon);

        glm::vec2 point_to_world_lat_lon(const glm::vec2 &point);

        void update_frustum(const glm::mat4 &projection_matrix,
                            const glm::mat4 &view_matrix);

        void calculate_tiles();

        void set_position(const glm::vec3 &position);

        void store_height_borders(const std::string &quadcode,
                                  const std::vector<unsigned short> &north_border,
                                  const std::vector<unsigned short> &south_border,
                                  const std::vector<unsigned short> &west_border,
                                  const std::vector<unsigned short> &east_border);

        const std::vector<std::shared_ptr<data_tile>> &get_tiles();

        const std::vector<std::shared_ptr<data_tile>> &get_height_tiles();

        uint8_t calculate_maximal_depth();

        void calculate_additional_tiles();

        bool targeted_screen_space_error(data_tile &tile, const uint8_t &depth);
    };
}

#endif //A_SIMPLE_TRIANGLE_WINDOWS_DATALAYER_HPP
