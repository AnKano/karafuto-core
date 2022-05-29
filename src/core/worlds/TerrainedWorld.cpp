#include "TerrainedWorld.hpp"
#include "stages/BuiltinStages.hpp"

#if defined(__APPLE__) || defined(__linux__) || defined(WINDOWS) || defined(WIN32)
//#include "../contexts/rendering/opencl/OpenCLRenderContext.hpp"
#include "../contexts/rendering/vulkan/VulkanRenderContext.hpp"
#elif defined(__EMSCRIPTEN__)
#include "../contexts/rendering/emscripten-webgl/WebGLRenderContext.hpp"
#else
#include "../contexts/rendering/fallback/FallbackRenderContext.hpp"
#endif

#include <algorithm>

namespace KCore {
    TerrainedWorld::TerrainedWorld() : BaseWorld(0.0f, 0.0f) {}

    TerrainedWorld::TerrainedWorld(float latitude, float longitude) : BaseWorld(latitude, longitude) {

#if defined(__APPLE__) || defined(__linux__) || defined(WINDOWS) || defined(WIN32)
        mRenderContext = new KCore::Vulkan::VulkanRenderContext(this);
//        mRenderContext = new KCore::OpenCL::OpenCLRenderContext(this);
#elif defined(__EMSCRIPTEN__)
        mRenderContext = new KCore::WebGL::WebGLRenderContext(this);
#else
        mRenderContext = new KCore::Fallback::FallbackRenderContext(this);
#endif

        registerStage(KCore::BuiltInStages::MetaCalculate());
    }

    void TerrainedWorld::calculateMetaTiles() {
        typedef std::chrono::high_resolution_clock Time;
        typedef std::chrono::milliseconds ms;
        typedef std::chrono::duration<float> fsec;

        auto t0 = Time::now();

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

        auto toRenderContext = std::vector<GenericTile *>();
        for (const auto &[quadcode, _]: mCurrMetaTiles)
            toRenderContext.push_back(mCreatedMetaTiles[quadcode]);
        mRenderContext->setCurrentTileState(toRenderContext);

        auto t1 = Time::now();
        fsec fs = t1 - t0;
        ms d = std::chrono::duration_cast<ms>(fs);
//        std::cout << "meta step: " << d.count() << "ms\n";

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
                if (mCreatedBaseTiles.count(quadcode) == 0) {
                    mCreatedBaseTiles[quadcode] = new GenericTile(this, item);
                    createBaseTileResources(mCreatedBaseTiles[quadcode]);
                    mCreatedBaseTiles[quadcode]->invokeResources();
                }
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
//        tile->registerImmediateResource("json", BuiltInResource::JSONWithTerrainAdaptation());
    }

    void TerrainedWorld::update() {
        BaseWorld::update();
        calculateMetaTiles();
        performStages();
    }

    IRenderContext *TerrainedWorld::getRenderContext() {
        return mRenderContext;
    }

#ifndef __EMSCRIPTEN__
    extern "C" {
    DllExport KCore::TerrainedWorld *CreateTerrainedWorld(float latitude, float longitude) {
        return new KCore::TerrainedWorld(latitude, longitude);
    }

    DllExport void CommitTerrainedWorld(KCore::TerrainedWorld *world) {
        world->commitWorldSetup();
    }

    DllExport void TerrainedWorldRegisterSource(KCore::TerrainedWorld *world, BaseSource *source, const char *tag) {
        world->getSources()[tag] = source;
    }
    }
#endif
}