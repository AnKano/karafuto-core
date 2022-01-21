#pragma once

#include <list>
#include <map>
#include <optional>

#include "glm/glm.hpp"

#include "../geography/TileDescription.hpp"
#include "../misc/FrustumCulling.hpp"
#include "BaseWorld.hpp"

namespace KCore {
    class TerrainedWorld : public BaseWorld {
        std::list<TileDescription> mMetaTiles{};

    public:
        TerrainedWorld(const glm::vec2 &originLatLon, const glm::vec2 &originPoint, const struct WorldConfig &config);

        void update() override;

        [[nodiscard]]
        const std::list<TileDescription> &getMetaTiles();

    private:
        void calculateTiles() override;

        void calculateMetaTiles();

        bool targetedScreenSpaceError(TileDescription &tile, const uint8_t &depth);

        uint8_t calculateMaximalDepth();
    };
}

