#pragma once

#include <array>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "glm/glm.hpp"

#include "GeographyConverter.hpp"

namespace KCore {
    class Layer;

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

    class TileDescription {
    private:
        std::string mQuadcode;

        glm::ivec3 mTilecode{};
        glm::vec4 mBoundsLatLon{};
        glm::vec4 mBoundsWorld{};
        glm::vec2 mCenter{};

        TileType mType = Leaf;
        TileVisibility mVisibility = Visible;

        float mScale{};

    public:
        TileDescription() = default;

        TileDescription
                (const Layer *parent, const std::string &quadcode);

        ~TileDescription() = default;

        void setQuadcode
                (const std::string &quadcode);

        void setTilecode
                (const glm::ivec3 &tilecode);

        void setBoundsLatLon
                (const glm::vec4 &boundsLatLon);

        void setBoundsWorld
                (const glm::vec4 &boundsWorld);

        void setCenter
                (const glm::vec2 &center);

        void setVisibility
                (const TileVisibility &visibility);

        void setScale
                (const float &scale);

        void setType
                (const TileType &type);

        [[nodiscard]] const std::string &getQuadcode() const;

        [[nodiscard]] const glm::ivec3 &getTilecode() const;

        [[nodiscard]] const glm::vec4 &getBoundsLatLon() const;

        [[nodiscard]] const glm::vec4 &getBoundsWorld() const;

        [[nodiscard]] const glm::vec2 &getCenter() const;

        [[nodiscard]] TileVisibility getVisibility() const;

        [[nodiscard]] float getScale() const;

        [[nodiscard]] TileType getType() const;

        [[nodiscard]] std::string createTileURL() const;
    };
}