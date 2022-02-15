#include "TerrainedWorld.hpp"

#include <algorithm>

namespace KCore {
//    TerrainedWorld::TerrainedWorld() : BaseWorld() {}
//
//    void TerrainedWorld::calculateTiles() {
//        BaseWorld::calculateTiles();
//    }
//
//    void TerrainedWorld::calculateMetaTiles() {
//        // previous meta tile now not actual, so remove it
//        mMetaTiles.clear();
//
//        auto depth = maximalCommonTilesDepth();
//        auto preferDepth = depth - 4;
//
//        std::map<std::string, TileDescription> collector;
//        std::map<std::string, std::vector<std::string>> childCollector;
//        std::map<std::string, std::vector<std::string>> parentCollector;
//        for (const auto &item: mBaseTiles) {
//            auto quadcode = item.getQuadcode();
//
//            if (quadcode.size() >= preferDepth) {
//                // calculate quadcode for associate with parent meta tile
//                auto preferedQuadcode = quadcode.substr(0, preferDepth);
//
//                // get tile description from cache
//                auto founded = mTilesCache.getByKey(preferedQuadcode);
//                if (founded) {
//                    collector[preferedQuadcode] = *founded;
//                    if (childCollector.find(preferedQuadcode) == std::end(childCollector))
//                        childCollector[preferedQuadcode] = {};
//                    childCollector[preferedQuadcode].push_back(item.getQuadcode());
//                } else {
//                    // !TODO: throw? maybe remove checking process?
//                    // impossible. if you are here that tile should be created
//                }
//            } else {
//                if (item.getVisibility() != Visible) continue;
//
//                auto parents = separateTileToDepth(item, preferDepth);
//                for (const auto &_item: parents) {
//                    auto internalQuadcode = _item.getQuadcode();
//                    collector[internalQuadcode] = _item;
//                    childCollector[internalQuadcode] = {};
//                    parentCollector[internalQuadcode] = {item.getQuadcode()};
//                }
//            }
//        }
//
//        for (const auto &[key, value]: collector)
//            mMetaTiles.push_back({value, childCollector[key], parentCollector[key]});
//    }
//
//
//    std::vector<TileDescription> TerrainedWorld::separateTileToDepth(const TileDescription &desc, uint8_t depth) {
//        auto parents = std::vector<TileDescription>();
//        const auto &rootQuadcode = desc.getQuadcode();
//
//        for (const auto &item: std::vector{"0", "1", "2", "3"}) {
//            auto founded = mTilesCache[rootQuadcode + item];
//            if (founded) {
//                parents.emplace_back(*founded);
//                continue;
//            }
//
//            auto tile = createTile(rootQuadcode + item);
//            auto element = mTilesCache.setOrReplace(rootQuadcode + item, tile);
//            parents.push_back(element);
//        }
//
//        std::size_t count{0};
//        while (count != parents.size()) {
//            auto tile = &parents[count];
//            auto quadcode = tile->getQuadcode();
//
//            if (checkTileInFrustum(*tile) && tile->getTilecode().z <= depth) {
//                if (tile->getType() != TileType::Root)
//                    tile->setType(TileType::Separated);
//                tile->setVisibility(TileVisibility::Hide);
//
//                for (const auto &item: std::vector{"0", "1", "2", "3"}) {
//                    auto founded = mTilesCache[quadcode + item];
//                    if (founded) {
//                        parents.emplace_back(*founded);
//                    } else {
//                        auto child = createTile(quadcode + item);
//                        auto tileDescription = mTilesCache.setOrReplace(quadcode + item, child);
//                        parents.push_back(tileDescription);
//                    }
//                }
//            }
//
//            count++;
//        }
//
//        // filter parents that reach last depth
//        auto condition = [&depth](const TileDescription &tile) {
//            return tile.getTilecode().z == depth;
//        };
//
//        auto result = std::vector<TileDescription>();
//        std::copy_if(parents.begin(), parents.end(), std::back_inserter(result), condition);
//        return result;
//    }
//
//    uint8_t TerrainedWorld::maximalCommonTilesDepth() {
//        uint8_t depth{0};
//        for (const auto &item: mBaseTiles)
//            if (item.getTilecode().z > depth)
//                depth = item.getTilecode().z;
//        return depth;
//    }
//
//    const std::vector<MetaTile> &TerrainedWorld::getMetaTiles() {
//        return mMetaTiles;
//    }
//
//    void TerrainedWorld::update() {
//        BaseWorld::update();
//
//        calculateMetaTiles();
//    }
}