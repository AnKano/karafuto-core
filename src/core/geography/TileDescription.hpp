#pragma once

#include "GeographyConverter.hpp"

#include "glm/glm.hpp"
#include <string>
#include <vector>
#include <optional>
#include <array>
#include <memory>
#include <map>

namespace KCore {
    class TileDescription;

    enum TileType {
        Root = 0,
        Separated = 1,
        Leaf = 2
    };

    enum TileCardinals {
        NorthWest = 0,
        NorthEast = 1,
        SouthWest = 2,
        SouthEast = 3
    };

    enum TileSides {
        North = 0,
        South = 1,
        West = 2,
        East = 3
    };

    enum TileVisibility {
        Hide = 0,
        Visible = 1
    };

    struct TilePayload {
        glm::ivec3 Tilecode{};                     /* 00..04..08..12 bytes */
        float Center[2]{};                         /* 12..16..20     bytes */
        float SideLength{};                        /* 20..24         bytes */
        TileType Type{Leaf};                       /* 24..28         bytes */
        TileVisibility Visibility{Visible};        /* 28..32         bytes */
    };

    struct TileChilds {
        std::array<TileDescription *, 4> InQuad{};
        std::array<std::array<TileDescription *, 2>, 4> Sides{};
    };

    class TileDescription {
    public:
        TilePayload mPayload;

    private:
        // all tile properties
        std::string mQuadcode;

        glm::ivec3 mTilecode{};
        glm::vec4 mBoundsLatLon{};
        glm::vec4 mBoundsWorld{};
        glm::vec2 mCenter{};
        glm::vec2 mCenterLatLon{};

        TileType mType = Leaf;
        TileVisibility mVisibility = Visible;

        float mSideLength{};

        TileCardinals mCardinal;

//        const TileDescription *mParent{};
//        TileChilds mChilds{};

        // variable data related to tile
        std::map<std::string, void *> mDataStash;

    public:
        TileDescription();

        TileDescription(const std::string &quadcode);

        ~TileDescription() = default;

//        void setParent(const TileDescription *parent_ptr);

        void setQuadcode(const std::string &quadcode);

        void setTilecode(const glm::ivec3 &tilecode);

        void setBoundsLatLon(const glm::vec4 &boundsLatLon);

        void setBoundsWorld(const glm::vec4 &boundsWorld);

        void setCenter(const glm::vec2 &center);

        void setVisibility(TileVisibility visibility);

        void setCenterLatLon(const glm::vec2 &centerLatLon);

        void setSideLength(float sideLength);

        void setType(TileType type);

        [[nodiscard]]
        const std::string &getQuadcode() const;

        [[nodiscard]]
        const glm::ivec3 &getTilecode() const;

        [[nodiscard]]
        const glm::vec4 &getBoundsLatLon() const;

        [[nodiscard]]
        const glm::vec4 &getBoundsWorld() const;

        [[nodiscard]]
        const glm::vec2 &getCenter() const;

        [[nodiscard]]
        TileVisibility getVisibility() const;

        [[nodiscard]]
        const glm::vec2 &getCenterLatLon() const;

        [[nodiscard]]
        float getSideLength() const;

        [[nodiscard]]
        TileType getType() const;

//        [[nodiscard]]
//        const TileChilds &getChilds() const;

    private:
        TileCardinals calculateCardinalFromQuadcode();
    };
}