#pragma once

#include "GeographyConverter.hpp"

#include "glm/glm.hpp"
#include <string>
#include <vector>
#include <optional>
#include <array>
#include <memory>

namespace KCore {
    class DataTile;

    enum DataTileType {
        Root = 0,
        Separated = 1,
        Leaf = 2
    };

    enum DataTileCardinals {
        NorthWest = 0,
        NorthEast = 1,
        SouthWest = 2,
        SouthEast = 3
    };

    enum DataTileSide {
        North = 0,
        South = 1,
        West = 2,
        East = 3
    };

    enum DataTileVisibility {
        Hide = 0,
        Visible = 1
    };

    struct DataTilePayload {
        glm::ivec3 Tilecode{};                     /* 0..4..8..12 */
        float Center[2]{};                         /* 12..16..20  */
        float SideLength{};                        /* 20..24      */
        DataTileType Type{Leaf};                   /* 24..28      */
        DataTileVisibility Visibility{Visible};    /* 28..32      */
    };

    struct DataTileChilds {
        std::array<DataTile*, 4> InQuad{};
        std::array<std::array<DataTile*, 2>, 4> Sides{};
    };

    class DataTile {
    private:
        DataTilePayload Payload;

        std::string Quadcode;

        glm::ivec3 Tilecode{};
        glm::vec4 BoundsLatLon{};
        glm::vec4 BoundsWorld{};
        glm::vec2 Center{};
        glm::vec2 CenterLatLon{};

        DataTileType Type{Leaf};
        DataTileVisibility Visibility{Visible};

        float SideLength{};

        const DataTile *Parent{};
        DataTileCardinals Cardinals;
    public:
        DataTileChilds Childs{};

        explicit DataTile(const std::string &quadcode);

        ~DataTile() = default;

        void setParent(const DataTile* parent_ptr);

        void setQuadcode(const std::string& quadcode);

        void setTilecode(const glm::ivec3& tilecode);

        void setBoundsLatLon(const glm::vec4& boundsLatLon);

        void setBoundsWorld(const glm::vec4& boundsWorld);

        void setCenter(const glm::vec2& center);

        void setVisibility(DataTileVisibility visibility);

        void setCenterLatLon(const glm::vec2 &centerLatLon);

        void setSideLength(float sideLength);

        enum DataTileCardinals getCardinalFromQuadcode();

        void setType(DataTileType type);

        [[nodiscard]]
        const std::string& getQuadcode() const;

        [[nodiscard]]
        const glm::ivec3& getTilecode() const;

        [[nodiscard]]
        const glm::vec4& getBoundsLatLon() const;

        [[nodiscard]]
        const glm::vec4& getBoundsWorld() const;

        [[nodiscard]]
        const glm::vec2& getCenter() const;

        [[nodiscard]]
        DataTileVisibility getVisibility() const;

        [[nodiscard]]
        const glm::vec2& getCenterLatLon() const;

        [[nodiscard]]
        float getSideLength() const;

        [[nodiscard]]
        DataTileType getType() const;
    };
}