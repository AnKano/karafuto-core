#include "TerrainedWorld.hpp"

#include <algorithm>

namespace KCore {
    TerrainedWorld::TerrainedWorld(const glm::vec2 &originLatLon,
                                   const glm::vec2 &originPoint,
                                   const struct WorldConfig &config) : BaseWorld(originLatLon, originPoint, config) {}

    void TerrainedWorld::calculateTiles() {
        // previous meta tile now not actual, so remove it
        mMetaTiles.clear();

        BaseWorld::calculateTiles();
    }

    void TerrainedWorld::calculateMetaTiles() {
        auto depth = calculateMaximalDepth();

        // create height nodes
        for (const auto &item: std::vector{"0", "1", "2", "3"})
            mMetaTiles.emplace_back(createTile(item, nullptr));

        std::size_t count = 0;
        while (count != mMetaTiles.size()) {
            // move iterator through list
            auto it = mMetaTiles.begin();
            std::advance(it, count);

            // unpack iterator
            auto tile = &(*it);
            auto quadcode = tile->getQuadcode();

            if (targetedScreenSpaceError(*tile, depth)) {
                if (tile->getType() != TileType::Root)
                    tile->setType(TileType::Separated);
                tile->setVisibility(TileVisibility::Hide);

                auto tileNW = findOrCreateTile(quadcode + "0", tile);
                auto tileNE = findOrCreateTile(quadcode + "1", tile);
                auto tileSW = findOrCreateTile(quadcode + "2", tile);
                auto tileSE = findOrCreateTile(quadcode + "3", tile);

                mMetaTiles.emplace_back(tileNW);
                mMetaTiles.emplace_back(tileNE);
                mMetaTiles.emplace_back(tileSW);
                mMetaTiles.emplace_back(tileSE);
            }

            count++;
        }
    }

    uint8_t TerrainedWorld::calculateMaximalDepth() {
        uint8_t depth{0};
        for (const auto &item: mCommonTiles)
            if (item.getTilecode().z > depth)
                depth = item.getTilecode().z;
        return depth;
    }

    bool TerrainedWorld::targetedScreenSpaceError(TileDescription &tile, const uint8_t &depth) {
        const auto &quadcode = tile.getQuadcode();

        if (!checkTileInFrustum(tile)) {
            // if it's not in frustum just hide it without separation
            tile.setVisibility(TileVisibility::Hide);
            return false;
        }

        return (depth - 3) >= tile.getTilecode().z;
    }

    const std::list<TileDescription> &TerrainedWorld::getMetaTiles() {
        return mMetaTiles;
    }

    void TerrainedWorld::update() {
        BaseWorld::update();

        calculateMetaTiles();
    }
}