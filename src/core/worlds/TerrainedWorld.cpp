#include "TerrainedWorld.hpp"

#include <algorithm>

namespace KCore {
    TerrainedWorld::TerrainedWorld(const glm::vec2 &originLatLon, const glm::vec2 &originPoint,
                                   const struct WorldConfig &config) :
            BaseWorld(originLatLon, originPoint, config) {}

    void TerrainedWorld::calculateTiles() {
        BaseWorld::calculateTiles();
    }

    void TerrainedWorld::calculateMetaTiles() {
        // previous meta tile now not actual, so remove it
        mMetaTiles.clear();

        auto depth = maximalCommonTilesDepth();
        auto preferDepth = depth - 3;

        std::map<std::string, TileDescription> collector;
        for (const auto &item: mCommonTiles) {
            auto quadcode = item.getQuadcode();
            auto preferedQuadcode = quadcode.substr(0, preferDepth);

            auto founded = mTilesCache.getByKey(preferedQuadcode);
            if (founded)
                collector[preferedQuadcode] = *founded;
        }

        for (const auto &[key, value]: collector)
            mMetaTiles.push_back(value);
    }

    uint8_t TerrainedWorld::maximalCommonTilesDepth() {
        uint8_t depth{0};
        for (const auto &item: mCommonTiles)
            if (item.getTilecode().z > depth)
                depth = item.getTilecode().z;
        return depth;
    }

    const std::vector<TileDescription> &TerrainedWorld::getMetaTiles() {
        return mMetaTiles;
    }

    void TerrainedWorld::update() {
        BaseWorld::update();

        calculateMetaTiles();
    }
}