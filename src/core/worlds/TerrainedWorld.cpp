#include "TerrainedWorld.hpp"
#include "stages/BuiltinStages.hpp"

#if defined(__APPLE__) || defined(__linux__) || defined(WINDOWS) || defined(WIN32)
//#include "../contexts/rendering/opencl/OpenCLRenderContext.hpp"
#include "../contexts/rendering/vulkan/VulkanRenderContext.hpp"
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
#else
        mRenderContext = new KCore::Fallback::FallbackRenderContext(this);
#endif

        registerStage(KCore::BuiltInStages::MetaCalculate());
    }

    void TerrainedWorld::calculateMetaTiles() {
        mPrevMetaTiles = std::move(mCurrMetaTiles);
        mCurrMetaTiles = {};

        auto hldDivision = divide(1.0);
        auto lldDivision = divide(2.5);

        std::map<std::string, std::vector<std::string>> childCollector;

        for (const auto &ll: lldDivision) {
            if (ll.getVisibility() != TileVisibility::Visible) continue;
            const auto &llQuadcode = ll.getQuadcode();

            std::vector<std::string> childs{};

            for (const auto &hl: hldDivision) {
                if (hl.getVisibility() != TileVisibility::Visible) continue;
                const auto &hlQuadcode = hl.getQuadcode();

                if (hlQuadcode == llQuadcode) continue;
                if (hlQuadcode.starts_with(llQuadcode))
                    childs.push_back(hlQuadcode);
            }

            childCollector[llQuadcode] = childs;
        }

        for (const auto &value: lldDivision) {
            if (value.getVisibility() != TileVisibility::Visible) continue;

            const auto &key = value.getQuadcode();

            if (mCreatedMetaTiles.count(key) == 0)
                mCreatedMetaTiles[key] = new GenericTile(this, value);
            mCreatedMetaTiles[key]->setChildQuadcodes(childCollector[key]);
            mCurrMetaTiles[key] = true;
        }

        auto toRenderContext = std::vector<GenericTile *>();
        for (const auto &[quadcode, _]: mCurrMetaTiles)
            toRenderContext.push_back(mCreatedMetaTiles[quadcode]);
        mRenderContext->setCurrentTileState(toRenderContext);

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
            const auto& quadcode = item.getQuadcode();
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
}