#include "TileDescription.hpp"

#include <iostream>
#include <sstream>

#include "../layer/Layer.hpp"

namespace KCore {
    TileDescription::TileDescription(const Layer *parent, const std::string &quadcode) {
        if (quadcode.empty()) throw std::invalid_argument("Provided empty quadcode!");
        setQuadcode(quadcode);

        setTilecode(GeographyConverter::quadcodeToTilecode(quadcode));

        {
            const auto &tilecode = getTilecode();

            auto w = GeographyConverter::tileToLon(tilecode[0], tilecode[2]);
            auto s = GeographyConverter::tileToLat(tilecode[1] + 1, tilecode[2]);
            auto e = GeographyConverter::tileToLon(tilecode[0] + 1, tilecode[2]);
            auto n = GeographyConverter::tileToLat(tilecode[1], tilecode[2]);

            setBoundsLatLon({w, s, e, n});
        }

        {
            const auto &boundsLatLon = getBoundsLatLon();

            auto sw = parent->latLonToWorldPosition({boundsLatLon[1], boundsLatLon[0]});
            auto ne = parent->latLonToWorldPosition({boundsLatLon[3], boundsLatLon[2]});

            setBoundsWorld({sw.x, sw.y, ne.x, ne.y});
        }

        {
            const auto &boundsWorld = getBoundsWorld();
            auto x = boundsWorld[0] + (boundsWorld[2] - boundsWorld[0]) / 2;
            auto y = boundsWorld[1] + (boundsWorld[3] - boundsWorld[1]) / 2;

            setCenter({x, y});
            setScale(boundsWorld[1] - boundsWorld[3]);
        }
    }

    std::string TileDescription::createTileURL() const {
        std::stringstream strstream;
        strstream << std::to_string(mTilecode.z) << "/"
                  << std::to_string(mTilecode.x) << "/"
                  << std::to_string(mTilecode.y);
        return strstream.str();
    }

    void TileDescription::setQuadcode(const std::string &quadcode) {
        TileDescription::mQuadcode = quadcode;
    }

    void TileDescription::setTilecode(const glm::ivec3 &tilecode) {
        TileDescription::mTilecode = tilecode;
    }

    void TileDescription::setBoundsLatLon(const glm::vec4 &boundsLatLon) {
        TileDescription::mBoundsLatLon = boundsLatLon;
    }

    void TileDescription::setBoundsWorld(const glm::vec4 &boundsWorld) {
        TileDescription::mBoundsWorld = boundsWorld;
    }

    void TileDescription::setCenter(const glm::vec2 &center) {
        TileDescription::mCenter = center;
    }

    void TileDescription::setScale(float scale) {
        TileDescription::mScale = scale;
    }

    void TileDescription::setType(TileType type) {
        TileDescription::mType = type;
    }

    void TileDescription::setVisibility(TileVisibility visibility) {
        TileDescription::mVisibility = visibility;
    }

    const std::string &TileDescription::getQuadcode() const {
        return mQuadcode;
    }

    const glm::ivec3 &TileDescription::getTilecode() const {
        return mTilecode;
    }

    const glm::vec4 &TileDescription::getBoundsLatLon() const {
        return mBoundsLatLon;
    }

    const glm::vec4 &TileDescription::getBoundsWorld() const {
        return mBoundsWorld;
    }

    const glm::vec2 &TileDescription::getCenter() const {
        return mCenter;
    }

    float TileDescription::getScale() const {
        return mScale;
    }

    TileType TileDescription::getType() const {
        return mType;
    }

    TileVisibility TileDescription::getVisibility() const {
        return mVisibility;
    }
}