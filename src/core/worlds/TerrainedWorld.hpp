#pragma once

#include <list>
#include <map>
#include <optional>

#include "glm/glm.hpp"

#include "BaseWorld.hpp"

#include "../geography/TileDescription.hpp"
#include "../misc/FrustumCulling.hpp"
#include "../cache/TimeoutCache.hpp"

namespace KCore {
    class TerrainedWorld : public BaseWorld {
        TimeoutCache<TileDescription> mTilesCache;
        std::vector<TileDescription> mMetaTiles;

    public:
        TerrainedWorld(const glm::vec2 &originLatLon, const glm::vec2 &originPoint,
                       const struct WorldConfig &config);

        void update() override;

        [[nodiscard]]
        const std::vector<TileDescription> &getMetaTiles();

    private:
        void calculateTiles() override;

        void calculateMetaTiles();

        bool targetedScreenSpaceError(TileDescription &tile, const uint8_t &depth);

        uint8_t maximalCommonTilesDepth();
    };
}

