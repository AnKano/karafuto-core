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
        std::vector<MetaTile> mMetaTiles{};

    public:
        TerrainedWorld(const glm::vec2 &originLatLon, const glm::vec2 &originPoint,
                       const struct WorldConfig &config);

        void update() override;

        [[nodiscard]]
        const std::vector<MetaTile> &getMetaTiles();

    private:
        std::vector<TileDescription> separateTileToDepth(const TileDescription& tile, uint8_t depth);

        void calculateTiles() override;

        void calculateMetaTiles();

        uint8_t maximalCommonTilesDepth();
    };
}

