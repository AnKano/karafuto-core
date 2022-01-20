//
// Created by Ash on 2/6/2021.
//

#include "DataWorld.hpp"

#include <algorithm>

namespace KCore {
    glm::vec2 DataWorld::latLonToGlPoint(const glm::vec2 &latLon) {
        auto projectedPoint = GeographyConverter::latLonToPoint(latLon);
        return projectedPoint - OriginPositionWGS84;
    }

    glm::vec2 DataWorld::glPointToLatLon(const glm::vec2 &point) {
        auto projectedPoint = point + OriginPositionWGS84;
        return GeographyConverter::pointToLatLon(projectedPoint);
    }

    DataWorld::DataWorld(const glm::vec2 &originLatLon, const glm::vec2 &originPoint)
            : OriginPositionWGS84(originPoint) {}

    void DataWorld::updateFrustum(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix) {
        Culling.update_frustum(projectionMatrix, viewMatrix);
    }

    DataTile DataWorld::createTile(const std::string &quadcode, const DataTile *parent) {
        DataTile tile(quadcode);

        tile.setParent(parent);
        tile.setTilecode(GeographyConverter::quadcodeToTilecode(quadcode));

        {
            auto tilecode = tile.getTilecode();

            auto w = GeographyConverter::tileToLon(tilecode[0], tilecode[2]);
            auto s = GeographyConverter::tileToLat(tilecode[1] + 1, tilecode[2]);
            auto e = GeographyConverter::tileToLon(tilecode[0] + 1, tilecode[2]);
            auto n = GeographyConverter::tileToLat(tilecode[1], tilecode[2]);

            tile.setBoundsLatLon({w, s, e, n});
        }

        {
            auto boundsLatLon = tile.getBoundsLatLon();

            auto sw = latLonToGlPoint({boundsLatLon[1], boundsLatLon[0]});
            auto ne = latLonToGlPoint({boundsLatLon[3], boundsLatLon[2]});

            tile.setBoundsWorld({sw.x, sw.y, ne.x, ne.y});
        }

        {
            auto boundsWorld = tile.getBoundsWorld();
            auto x = boundsWorld[0] + (boundsWorld[2] - boundsWorld[0]) / 2;
            auto y = boundsWorld[1] + (boundsWorld[3] - boundsWorld[1]) / 2;

            tile.setCenter({x, y});

            auto firstPoint = glm::vec3(boundsWorld[0], 0, boundsWorld[3]);
            auto secondPoint = glm::vec3(boundsWorld[0], 0, boundsWorld[1]);

            tile.setSideLength(glm::length(firstPoint - secondPoint));
            tile.setCenterLatLon(glPointToLatLon(tile.getCenter()));
        }

        return tile;
    }

    DataTile DataWorld::findOrCreateTile(const std::string &quadcode, const DataTile *parent) {
        if (TileTree.find(quadcode) != TileTree.end())
            return *TileTree[quadcode];

        return createTile(quadcode, parent);
    }

    void DataWorld::calculateTiles() {
        // clear up tile tree
        TileTree.clear();

        // clear up all Tiles
        Tiles.clear();

        // prevoius meta Tiles not actuall, so clean it either
        MetaTiles.clear();

        // create root nodes
        Tiles.emplace_back(createTile("0", nullptr));
        Tiles.emplace_back(createTile("1", nullptr));
        Tiles.emplace_back(createTile("2", nullptr));
        Tiles.emplace_back(createTile("3", nullptr));

        std::size_t count{0};
        while (count != Tiles.size()) {
            // move iterator through list
            auto it = Tiles.begin();
            std::advance(it, count);

            // unpack iterator
            auto tile = &(*it);
            auto quadcode = tile->getQuadcode();

            TileTree[quadcode] = tile;

            if (screenSpaceError(*tile, 3.0)) {
                if (tile->getType() != DataTileType::Root)
                    tile->setType(DataTileType::Separated);
                tile->setVisibility(DataTileVisibility::Hide);

                auto tileNW = createTile(quadcode + "0", tile);
                auto tileNE = createTile(quadcode + "1", tile);
                auto tileSW = createTile(quadcode + "2", tile);
                auto tileSE = createTile(quadcode + "3", tile);

                Tiles.emplace_back(tileNW);
                Tiles.emplace_back(tileNE);
                Tiles.emplace_back(tileSW);
                Tiles.emplace_back(tileSE);

                auto tileNW_ptr = &(tileNW);
                auto tileNE_ptr = &(tileNE);
                auto tileSW_ptr = &(tileSW);
                auto tileSE_ptr = &(tileSE);

                tile->Childs.InQuad[NorthWest] = tileNW_ptr;
                tile->Childs.InQuad[NorthEast] = tileNE_ptr;
                tile->Childs.InQuad[SouthWest] = tileSW_ptr;
                tile->Childs.InQuad[SouthEast] = tileSE_ptr;

                tile->Childs.Sides[North] = {tileNW_ptr, tileNE_ptr};
                tile->Childs.Sides[South] = {tileSW_ptr, tileSE_ptr};
                tile->Childs.Sides[West] = {tileNW_ptr, tileSW_ptr};
                tile->Childs.Sides[East] = {tileNE_ptr, tileSE_ptr};
            }

            count++;
        }
    }

    void DataWorld::calculateMetaTiles() {
        auto depth = calculateMaximalDepth();

        // create height nodes
        MetaTiles.emplace_back(findOrCreateTile("0", nullptr));
        MetaTiles.emplace_back(findOrCreateTile("1", nullptr));
        MetaTiles.emplace_back(findOrCreateTile("2", nullptr));
        MetaTiles.emplace_back(findOrCreateTile("3", nullptr));

        std::size_t count = 0;
        while (count != MetaTiles.size()) {
            // move iterator through list
            auto it = MetaTiles.begin();
            std::advance(it, count);

            // unpack iterator
            auto tile = &(*it);
            auto quadcode = tile->getQuadcode();

            if (targetedScreenSpaceError(*tile, depth)) {
                if (tile->getType() != DataTileType::Root)
                    tile->setType(DataTileType::Separated);
                tile->setVisibility(DataTileVisibility::Hide);

                auto tileNW = findOrCreateTile(quadcode + "0", tile);
                auto tileNE = findOrCreateTile(quadcode + "1", tile);
                auto tileSW = findOrCreateTile(quadcode + "2", tile);
                auto tileSE = findOrCreateTile(quadcode + "3", tile);

                MetaTiles.emplace_back(tileNW);
                MetaTiles.emplace_back(tileNE);
                MetaTiles.emplace_back(tileSW);
                MetaTiles.emplace_back(tileSE);
            }

            count++;
        }
    }

    uint8_t DataWorld::calculateMaximalDepth() {
        uint8_t depth{0};
        for (const auto &item: Tiles)
            if (item.getTilecode().z > depth)
                depth = item.getTilecode().z;
        return depth;
    }

    bool DataWorld::targetedScreenSpaceError(DataTile &tile, const uint8_t &depth) {
        const auto &quadcode = tile.getQuadcode();

        if (!checkTileInFrustum(tile)) {
            // if it's not in frustum just hide it without separation
            tile.setVisibility(DataTileVisibility::Hide);
            return false;
        }

        return (depth - 3) >= tile.getTilecode().z;
    }

    bool DataWorld::screenSpaceError(DataTile &tile, float quality = 3.0f) {
        const auto &quadcode = tile.getQuadcode();

        if (!checkTileInFrustum(tile)) {
            // if it's not in frustum just hide it without separation
            tile.setVisibility(DataTileVisibility::Hide);
            return false;
        }

        auto center = tile.getCenter();
        auto distance = glm::length(glm::vec3(center.x, 0, center.y) - OriginPositionOGL);
        auto error = quality * tile.getSideLength() / distance;

        return error > 1.0f;
    }

    bool DataWorld::checkTileInFrustum(const DataTile &tile) {
        auto pos = tile.getCenter();
        auto scale = tile.getSideLength();

        float minX = (float) pos.x - scale, maxX = (float) pos.x + scale;
        float minZ = (float) pos.y - scale, maxZ = (float) pos.y + scale;
        float minY = 0.0f, maxY = 0.0f;

        auto result = Culling.test_box(minX, minY, minZ, maxX, maxY, maxZ);
        return result;
    }

    void DataWorld::setPosition(const glm::vec3 &position) {
        OriginPositionOGL = position;
    }

    const std::list<DataTile> &DataWorld::getTiles() {
        return Tiles;
    }

    const std::list<DataTile> &DataWorld::getMetaTiles() {
        return MetaTiles;
    }
}