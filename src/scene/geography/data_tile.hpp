#pragma once

#include "geo_converter.hpp"

#include "glm/glm.hpp"
#include <string>
#include <vector>
#include <optional>
#include <array>
#include <memory>

namespace kcore {
    class data_tile;

    enum data_tile_type {
        root = 0,
        separated = 1,
        leaf = 2
    };

    enum data_tile_cardinals {
        north_west = 0,
        north_east = 1,
        south_west = 2,
        south_east = 3
    };

    enum data_tile_side {
        north = 0,
        south = 1,
        west = 2,
        east = 3
    };

    enum data_tile_visibility {
        hide = 0,
        visible = 1
    };

    // needed for clear data access
    struct data_tile_payload {
        glm::ivec3 tile_code{};                      /* 0..4..8..12 */
        float center[2]{};                           /* 12..16..20  */
        float side_length{};                         /* 20..24      */
        data_tile_type type{leaf};                   /* 24..28      */
        data_tile_visibility visibility{visible};    /* 28..32      */
    };

    struct data_tile_childs {
        std::array<data_tile*, 4> quad{};
        std::array<std::array<data_tile*, 2>, 4> sides{};
    };

    class data_tile {
    private:
        data_tile_payload payload;

        std::string quadcode;

        glm::ivec3 tilecode{};
        glm::vec4 bounds_lat_lon{};
        glm::vec4 bounds_world{};
        glm::vec2 center{};
        glm::vec2 center_lat_lon{};

        data_tile_type type{leaf};
        data_tile_visibility visibility{visible};

        float side_length{};

        const data_tile *parent{};
        data_tile_cardinals cardinals;
    public:
        data_tile_childs childs{};

        explicit data_tile(const std::string &quadcode);

        ~data_tile() = default;

        void set_parent(const data_tile* parent);

        void set_quadcode(const std::string& quadcode);

        void set_tilecode(const glm::ivec3& tilecode);

        void set_bounds_lat_lon(const glm::vec4& bounds_lat_lon);

        void set_bounds_world(const glm::vec4& bounds_world);

        void set_center(const glm::vec2& center);

        void set_visibility(data_tile_visibility visibility);

        void set_center_lat_lon(const glm::vec2 &center_lat_lon);

        void set_side_length(float side_length);

        enum data_tile_cardinals eject_cardinals();

        void set_type(data_tile_type type);

        [[nodiscard]]
        const std::string& get_quadcode() const;

        [[nodiscard]]
        const glm::ivec3& get_tilecode() const;

        [[nodiscard]]
        const glm::vec4& get_bounds_lat_lon() const;

        [[nodiscard]]
        const glm::vec4& get_bounds_world() const;

        [[nodiscard]]
        const glm::vec2& get_center() const;

        [[nodiscard]]
        data_tile_visibility get_visibility() const;

        [[nodiscard]]
        const glm::vec2& get_center_lat_lon() const;

        [[nodiscard]]
        float get_side_length() const;

        [[nodiscard]]
        data_tile_type get_type() const;
    };
}