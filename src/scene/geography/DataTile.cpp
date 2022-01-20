//
// Created by Ash on 2/6/2021.
//

#include "DataTile.hpp"

#include <iostream>

namespace KCore {
    DataTile::DataTile(const std::string &quadcode) {
        if (quadcode.empty())
            throw std::invalid_argument("empty Quadcode");

        setQuadcode(quadcode);
        Cardinals = getCardinalFromQuadcode();
    }

    void DataTile::setQuadcode(const std::string& quadcode) {
        DataTile::Quadcode = quadcode;
    }

    void DataTile::setParent(const DataTile *parent_ptr) {
        DataTile::Parent = parent_ptr;
        setType((parent_ptr == nullptr) ? DataTileType::Root : DataTileType::Leaf);
    }

    void DataTile::setTilecode(const glm::ivec3 &tilecode) {
        DataTile::Payload.Tilecode.x = static_cast<int32_t>(tilecode.x);
        DataTile::Payload.Tilecode.y = static_cast<int32_t>(tilecode.y);
        DataTile::Payload.Tilecode.z = static_cast<int32_t>(tilecode.z);
        DataTile::Tilecode = tilecode;
    }

    void DataTile::setBoundsLatLon(const glm::vec4 &boundsLatLon) {
        DataTile::BoundsLatLon = boundsLatLon;
    }

    void DataTile::setBoundsWorld(const glm::vec4 &boundsWorld) {
        DataTile::BoundsWorld = boundsWorld;
    }

    void DataTile::setCenter(const glm::vec2 &center) {
        DataTile::Payload.Center[0] = center.x;
        DataTile::Payload.Center[1] = center.y;
        DataTile::Center = center;
    }

    void DataTile::setCenterLatLon(const glm::vec2 &centerLatLon) {
        DataTile::CenterLatLon = centerLatLon;
    }

    void DataTile::setSideLength(float sideLength) {
        DataTile::Payload.SideLength = sideLength;
        DataTile::SideLength = sideLength;
    }

    void DataTile::setType(DataTileType type) {
        DataTile::Payload.Type = type;
        DataTile::Type = type;
    }

    void DataTile::setVisibility(DataTileVisibility visibility) {
        DataTile::Payload.Visibility = visibility;
        DataTile::Visibility = visibility;
    }

    const std::string &DataTile::getQuadcode() const {
        return Quadcode;
    }

    const glm::ivec3 &DataTile::getTilecode() const {
        return Tilecode;
    }

    const glm::vec4 &DataTile::getBoundsLatLon() const {
        return BoundsLatLon;
    }

    const glm::vec4 &DataTile::getBoundsWorld() const {
        return BoundsWorld;
    }

    const glm::vec2 &DataTile::getCenter() const {
        return Center;
    }

    const glm::vec2 &DataTile::getCenterLatLon() const {
        return CenterLatLon;
    }

    float DataTile::getSideLength() const {
        return SideLength;
    }

    DataTileType DataTile::getType() const {
        return Type;
    }

    DataTileVisibility DataTile::getVisibility() const {
        return Visibility;
    }

    enum DataTileCardinals DataTile::getCardinalFromQuadcode() {
        switch (Quadcode.back()) {
            case '0': return NorthWest;
            case '1': return NorthEast;
            case '2': return SouthWest;
            case '3': return SouthEast;
            default:
                throw std::runtime_error("Unexpected tile Cardinals");
        }
    }
}