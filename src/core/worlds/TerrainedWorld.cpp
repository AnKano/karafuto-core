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
        auto depth = maximalCommonTilesDepth();

        // create height nodes
        for (const auto &item: std::vector{"0", "1", "2", "3"}) {
            auto founded = mTilesCache[item];
            if (founded != std::nullopt) {
                mMetaTiles.emplace_back(founded.value());
                continue;
            }

            auto tile = createTile(item);
            auto element = mTilesCache.setOrReplace(item, tile);
            mMetaTiles.push_back(element);
        }

        std::size_t count = 0;
        while (count != mMetaTiles.size()) {
            auto tile = mMetaTiles[count];
            auto quadcode = tile.getQuadcode();

            if (targetedScreenSpaceError(tile, depth)) {
                if (tile.getType() != TileType::Root)
                    tile.setType(TileType::Separated);
                tile.setVisibility(TileVisibility::Hide);

                auto founded = mTilesCache[quadcode + "0"];
                if (founded != std::nullopt) {
                    mMetaTiles.push_back(founded.value());
                } else {
                    auto tileNW = createTile(quadcode + "0");
                    auto tileDescription = mTilesCache.setOrReplace(quadcode + "0", tileNW);
                    mMetaTiles.push_back(tileDescription);
                }

                founded = mTilesCache[quadcode + "1"];
                if (founded != std::nullopt) {
                    mMetaTiles.push_back(founded.value());
                } else {
                    auto tileNE = createTile(quadcode + "1");
                    auto tileDescription = mTilesCache.setOrReplace(quadcode + "1", tileNE);
                    mMetaTiles.push_back(tileDescription);
                }

                founded = mTilesCache[quadcode + "2"];
                if (founded != std::nullopt) {
                    mMetaTiles.push_back(founded.value());
                } else {
                    auto tileSW = createTile(quadcode + "2");
                    auto tileDescription = mTilesCache.setOrReplace(quadcode + "2", tileSW);
                    mMetaTiles.push_back(tileDescription);
                }

                founded = mTilesCache[quadcode + "3"];
                if (founded != std::nullopt) {
                    mMetaTiles.push_back(founded.value());
                } else {
                    auto tileSE = createTile(quadcode + "3");
                    auto tileDescription = mTilesCache.setOrReplace(quadcode + "3", tileSE);
                    mMetaTiles.push_back(tileDescription);
                }
            }

            count++;
        }
    }

    uint8_t TerrainedWorld::maximalCommonTilesDepth() {
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

    const std::vector<TileDescription> &TerrainedWorld::getMetaTiles() {
        return mMetaTiles;
    }

    void TerrainedWorld::update() {
        BaseWorld::update();

        calculateMetaTiles();
    }
}