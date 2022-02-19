#include "TerrainedWorld.hpp"
#include "stages/BuiltinStages.hpp"

#include <algorithm>

namespace KCore {
    TerrainedWorld::TerrainedWorld() : BaseWorld(0.0f, 0.0f) {}

    TerrainedWorld::TerrainedWorld(float latitude, float longitude) : BaseWorld(latitude, longitude) {
        registerStage(KCore::BuiltInStages::MetaCalculate());
    }

    void TerrainedWorld::calculateMetaTiles() {
        // store old tiles and clear up current
        mPrevMetaTiles = std::move(mCurrMetaTiles);
        mCurrMetaTiles = {};

        auto depth = maximalCommonTilesDepth();
        auto preferDepth = depth - 4;

        std::map<std::string, TileDescription> collector;
        std::map<std::string, std::vector<std::string>> childCollector;
        std::map<std::string, std::vector<std::string>> parentCollector;

        for (const auto &[quadcode, _]: mCurrBaseTiles) {
            auto tile = mCreatedBaseTiles[quadcode]->getTileDescription();

            if (quadcode.size() >= preferDepth) {
                // calculate quadcode for associate with parent meta tile
                auto preferedQuadcode = quadcode.substr(0, preferDepth);

                // get tile description from cache
                collector[preferedQuadcode] = createTile(preferedQuadcode);
                childCollector[preferedQuadcode].push_back(quadcode);
            } else {
                if (tile.getVisibility() != Visible) continue;

                auto parents = separateTileToDepth(tile, preferDepth);
                for (const auto &_item: parents) {
                    auto internalQuadcode = _item.getQuadcode();
                    collector[internalQuadcode] = _item;
                    childCollector[internalQuadcode] = {};
                    parentCollector[internalQuadcode] = {quadcode};
                }
            }
        }

        for (const auto &[key, value]: collector) {
            if (mCreatedMetaTiles.count(key) == 0)
                mCreatedMetaTiles[key] = new GenericTile(this, value);
            mCreatedMetaTiles[key]->setChildQuadcodes(childCollector[key]);
            mCreatedMetaTiles[key]->setParentQuadcodes(parentCollector[key]);
            mCurrMetaTiles[key] = true;
        }

        auto toRenderContext = std::vector<GenericTile*>();
        for (const auto &[quadcode, _]: mCurrMetaTiles)
            toRenderContext.push_back(mCreatedMetaTiles[quadcode]);
        mRenderContext.setCurrentTileState(toRenderContext);

        postMetaTileCalculation();
    }

    void TerrainedWorld::postTileCalculation(const std::vector<TileDescription> &tiles) {
        auto condition = [this](const TileDescription &tile) {
            if (tile.getVisibility() != Visible) return false;
            const auto center = tile.getCenter();
            auto distance = glm::length(glm::vec3(center.x, 0, center.y) - mOriginPosition);
            return distance <= 500000.0f;
        };

        for (const auto &item: tiles) {
            auto quadcode = item.getQuadcode();
            if (condition(item)) {
                if (mCreatedBaseTiles.count(quadcode) == 0)
                    mCreatedBaseTiles[quadcode] = new GenericTile(this, item);
                createBaseTileResources(mCreatedBaseTiles[quadcode]);
                mCreatedBaseTiles[quadcode]->invokeResources();
                mCurrBaseTiles[quadcode] = true;
            }
        }
    }

    void TerrainedWorld::postMetaTileCalculation() {
        for (const auto &[quadcode, _]: mCurrMetaTiles) {
            auto &tile = mCreatedMetaTiles[quadcode];

            if (!tile->mInvoked) {
                createMetaTileResources(tile);
                tile->invokeResources();
            }
        }
    }

    std::vector<TileDescription> TerrainedWorld::separateTileToDepth(const TileDescription &desc, uint8_t depth) {
        auto parents = std::vector<TileDescription>();
        const auto &rootQuadcode = desc.getQuadcode();

        for (const auto &item: std::vector{"0", "1", "2", "3"}) {
            auto tile = createTile(rootQuadcode + item);
            parents.push_back(tile);
        }

        std::size_t count{0};
        while (count != parents.size()) {
            auto tile = &parents[count];
            auto quadcode = tile->getQuadcode();

            if (checkTileInFrustum(*tile) && tile->getTilecode().z <= depth) {
                if (tile->getType() != TileType::Root)
                    tile->setType(TileType::Separated);
                tile->setVisibility(TileVisibility::Hide);

                for (const auto &item: std::vector{"0", "1", "2", "3"}) {
                    auto child = createTile(quadcode + item);
                    parents.push_back(child);
                }
            }

            count++;
        }

        // filter parents that reach last depth
        auto condition = [&depth](const TileDescription &tile) {
            return tile.getTilecode().z == depth;
        };

        auto result = std::vector<TileDescription>();
        std::copy_if(parents.begin(), parents.end(), std::back_inserter(result), condition);
        return result;
    }

    uint8_t TerrainedWorld::maximalCommonTilesDepth() {
        uint8_t depth{0};
        for (const auto &[quadcode, _]: mCurrBaseTiles) {
            auto tile = mCreatedBaseTiles[quadcode]->getTileDescription();
            if (tile.getTilecode().z > depth)
                depth = tile.getTilecode().z;
        }
        return depth;
    }

    void TerrainedWorld::performStages() {
        mSyncEvents.clear();

        for (const auto &item: mStages)
            item->invoke(this);

        mIteration++;
    }

    void TerrainedWorld::createBaseTileResources(GenericTile *tile) {
        tile->registerImmediateResource("image", BuiltInResource::ImageCalculateMeta());
    }

    void TerrainedWorld::createMetaTileResources(GenericTile *tile) {
        tile->registerImmediateResource("terrain", BuiltInResource::TerrainCalculate());
    }

    void TerrainedWorld::update() {
        BaseWorld::update();
        calculateMetaTiles();
        performStages();
    }

    RenderContext &TerrainedWorld::getRenderContext() {
        return mRenderContext;
    }

    extern "C" {
    DllExport KCore::TerrainedWorld *CreateTerrainWorld(float latitude, float longitude) {
        return new KCore::TerrainedWorld(latitude, longitude);
    }

    DllExport void TerrainedWorldRegisterSource(KCore::TerrainedWorld *world, BaseSource *source, const char *tag) {
        world->getSources()[tag] = source;
    }
    }
}