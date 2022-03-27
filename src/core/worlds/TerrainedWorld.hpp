#pragma once

#include <list>
#include <map>
#include <optional>

#include "glm/glm.hpp"

#include "BaseWorld.hpp"

#include "../geography/TileDescription.hpp"
#include "../misc/FrustumCulling.hpp"
#include "../cache/TimeoutCache.hpp"
#include "../geography/tiles/MetaTile.hpp"
#ifndef __EMSCRIPTEN__
#include "../contexts/rendering/IRenderContext.hpp"
#endif

namespace KCore {
    class TerrainedWorld : public BaseWorld {
    private:
#ifndef __EMSCRIPTEN__
        IRenderContext *mRenderContext;
#endif
    public:
        TerrainedWorld();

        TerrainedWorld(float latitude, float longitude);

        void update() override;

#ifndef __EMSCRIPTEN__
        IRenderContext *getRenderContext();
#endif

        void commitWorldSetup() {
            auto gen = GenericTile(this, createTile("0"));
            BuiltInResource::JSONWithTerrainAdaptation()(this, &gen);
        }

    private:

        std::vector<TileDescription> separateTileToDepth(const TileDescription &tile, uint8_t depth);

        void calculateMetaTiles();

        uint8_t maximalCommonTilesDepth();

    protected:
        void performStages() override;

        void createBaseTileResources(GenericTile *tile) override;

        void createMetaTileResources(GenericTile *tile);

        void postTileCalculation(const std::vector<TileDescription> &tiles) override;

        void postMetaTileCalculation() override;
    };

#ifndef __EMSCRIPTEN__
    extern "C" {
    DllExport KCore::TerrainedWorld *CreateTerrainedWorld(float latitude, float longitude);

    DllExport void CommitTerrainedWorld(KCore::TerrainedWorld *world);

    DllExport void TerrainedWorldRegisterSource(KCore::TerrainedWorld *world, BaseSource *source, const char *tag);
    }
#endif
}

