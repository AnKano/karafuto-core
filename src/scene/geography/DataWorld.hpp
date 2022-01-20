#pragma once

#include <list>
#include <map>
#include <optional>

#include "glm/glm.hpp"

#include "DataTile.hpp"
#include "../misc/FrustumCulling.hpp"

namespace KCore {
    class DataWorld {
        KCore::FrustumCulling Culling{};

        std::list<DataTile> Tiles{};
        std::list<DataTile> MetaTiles{};

        std::map<std::string, DataTile *> TileTree;

        glm::vec2 OriginPositionWGS84{};
        glm::vec3 OriginPositionOGL{};

        bool screenSpaceError(DataTile &tile, float quality);

        bool targetedScreenSpaceError(DataTile &tile, const uint8_t &depth);

        bool checkTileInFrustum(const DataTile &tile);

        DataTile createTile(const std::string &quadcode, const DataTile *parent);

        DataTile findOrCreateTile(const std::string &quadcode, const DataTile *parent);

        uint8_t calculateMaximalDepth();

    public:
        DataWorld(const glm::vec2 &originLatLon, const glm::vec2 &originPoint);

        glm::vec2 latLonToGlPoint(const glm::vec2 &latLon);

        glm::vec2 glPointToLatLon(const glm::vec2 &point);

        void updateFrustum(const glm::mat4 &projectionMatrix,
                           const glm::mat4 &viewMatrix);

        void setPosition(const glm::vec3 &position);

        const std::list<DataTile> &getTiles();

        const std::list<DataTile> &getMetaTiles();

        void calculateTiles();

        void calculateMetaTiles();
    };
}

