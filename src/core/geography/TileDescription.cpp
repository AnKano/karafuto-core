#include "TileDescription.hpp"

#include <iostream>
#include <cstring>

namespace KCore {
    TileDescription::TileDescription() = default;

    TileDescription::TileDescription(const std::string &quadcode) {
        if (quadcode.empty())
            throw std::invalid_argument("Provided empty quadcode");

        setQuadcode(quadcode);
        mCardinal = calculateCardinalFromQuadcode();
    }

    std::string TileDescription::tileURL() const {
        return {std::to_string(mTilecode.z) + "/" +
                std::to_string(mTilecode.x) + "/" +
                std::to_string(mTilecode.y)};
    }

    void TileDescription::setQuadcode(const std::string &quadcode) {
#if defined(_MSC_VER)
        strcpy_s(TileDescription::mPayload.Quadcode, quadcode.c_str());
#endif
#if defined(__GNUC__)
        strcpy(TileDescription::mPayload.Quadcode, quadcode.c_str());
#endif
        TileDescription::mQuadcode = quadcode;
    }

    void TileDescription::setTilecode(const glm::ivec3 &tilecode) {
        TileDescription::mPayload.Tilecode.x = static_cast<int32_t>(tilecode.x);
        TileDescription::mPayload.Tilecode.y = static_cast<int32_t>(tilecode.y);
        TileDescription::mPayload.Tilecode.z = static_cast<int32_t>(tilecode.z);
        TileDescription::mTilecode = tilecode;
        TileDescription::mTilecodeStr = tileURL();
    }

    void TileDescription::setBoundsLatLon(const glm::vec4 &boundsLatLon) {
        TileDescription::mBoundsLatLon = boundsLatLon;
    }

    void TileDescription::setBoundsWorld(const glm::vec4 &boundsWorld) {
        TileDescription::mBoundsWorld = boundsWorld;
    }

    void TileDescription::setCenter(const glm::vec2 &center) {
        TileDescription::mPayload.Center[0] = center.x;
        TileDescription::mPayload.Center[1] = center.y;
        TileDescription::mCenter = center;
    }

    void TileDescription::setCenterLatLon(const glm::vec2 &centerLatLon) {
        TileDescription::mCenterLatLon = centerLatLon;
    }

    void TileDescription::setSideLength(float sideLength) {
        TileDescription::mPayload.SideLength = sideLength;
        TileDescription::mSideLength = sideLength;
    }

    void TileDescription::setType(TileType type) {
        TileDescription::mPayload.Type = type;
        TileDescription::mType = type;
    }

    void TileDescription::setVisibility(TileVisibility visibility) {
        TileDescription::mPayload.Visibility = visibility;
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

    const glm::vec2 &TileDescription::getCenterLatLon() const {
        return mCenterLatLon;
    }

    float TileDescription::getSideLength() const {
        return mSideLength;
    }

    TileType TileDescription::getType() const {
        return mType;
    }

    TileVisibility TileDescription::getVisibility() const {
        return mVisibility;
    }

    enum TileCardinals TileDescription::calculateCardinalFromQuadcode() {
        switch (mQuadcode.back()) {
            case '0':
                return NorthWest;
            case '1':
                return NorthEast;
            case '2':
                return SouthWest;
            case '3':
                return SouthEast;
            default:
                throw std::runtime_error("Unexpected tile mCardinal");
        }
    }
}