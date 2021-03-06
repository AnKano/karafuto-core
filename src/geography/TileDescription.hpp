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

    enum TileVisibility {
        Hide = 0,
        Visible = 1
    };

    struct TilePayload {
        glm::ivec3 Tilecode{0, 0, 0};              /* 00..04..08..12 bytes */
        float Center[2]{0.0f, 0.0f};               /* 12..16..20     bytes */
        float SideLength{0.0f};                    /* 20..24         bytes */
        TileType Type{Leaf};                       /* 24..28         bytes */
        TileVisibility Visibility{Visible};        /* 28..32         bytes */
        char Quadcode[32];                         /* 32..64         bytes */
    };

    class TileDescription {
    public:
        TilePayload mPayload;

    private:
        std::string mQuadcode;
        std::vector<std::string> mRelatedQuadcodes;

        glm::ivec3 mTilecode{};
        glm::vec4 mBoundsLatLon{};
        glm::vec4 mBoundsWorld{};
        glm::vec2 mCenter{};

        TileType mType = Leaf;
        TileVisibility mVisibility = Visible;

        float mSideLength{};

    public:
        TileDescription();

        TileDescription(const std::string &quadcode);

        ~TileDescription() = default;

        [[nodiscard]]
        std::string tileURL() const;

        void setQuadcode(const std::string &quadcode);

        void setRelatedQuadcodes(const std::vector<std::string> &quadcodes);

        void setTilecode(const glm::ivec3 &tilecode);

        void setBoundsLatLon(const glm::vec4 &boundsLatLon);

        void setBoundsWorld(const glm::vec4 &boundsWorld);

        void setCenter(const glm::vec2 &center);

        void setVisibility(TileVisibility visibility);

        void setSideLength(float sideLength);

        void setType(TileType type);

        [[nodiscard]]
        const std::string &getQuadcode() const;

        [[nodiscard]]
        const std::vector<std::string> &getRelatedQuadcodes() const;

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
        float getSideLength() const;

        [[nodiscard]]
        TileType getType() const;
    };
}