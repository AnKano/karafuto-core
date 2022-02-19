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

namespace KCore {
    class TerrainedWorld : public BaseWorld {
    public:
        TerrainedWorld();

        TerrainedWorld(float latitude, float longitude);

        void update() override;

    private:
        std::vector<TileDescription> separateTileToDepth(const TileDescription &tile, uint8_t depth);

        void calculateMetaTiles();

        uint8_t maximalCommonTilesDepth();

    protected:
        void performStages() override;

        void createTileResources(GenericTile *tile) override;

        void postTileCalculation(const std::vector<TileDescription> &tiles) override;

        void postMetaTileCalculation() override;
    };

    extern "C" {
    DllExport KCore::TerrainedWorld *CreateTerrainedWorld(float latitude, float longitude);

    DllExport void TerrainedWorldRegisterSource(KCore::TerrainedWorld *world, BaseSource *source, const char *tag);
    }
}

