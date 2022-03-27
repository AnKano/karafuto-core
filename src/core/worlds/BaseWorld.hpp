#pragma once

#include <unordered_map>
#include <map>

#include "../misc/FrustumCulling.hpp"
#include "../geography/TileDescription.hpp"
#include "../cache/TimeoutCache.hpp"
#include "../contexts/task/TaskContext.hpp"

#ifndef __EMSCRIPTEN__
#include "../contexts/network/NetworkContext.hpp"
#endif

#include "../sources/BaseSource.hpp"
#include "../geography/tiles/GenericTile.hpp"
#include "stages/Stage.hpp"
#include "../sources/RemoteSource.hpp"
#include "../events/MapEvent.hpp"
#include "../misc/STBImageUtils.hpp"
#include "../sources/local/geojson/GeoJSONObject.hpp"
#include "../sources/local/geojson/primitives/GeoJSONTransObject.hpp"
#include "../meshes/PolygonMesh.hpp"
#include "../meshes/PolylineMesh.hpp"
#include "../geography/tiles/resource/BuiltInResource.hpp"

namespace KCore {
    class BaseWorld {
    public:
        int mIteration = 0;

    protected:
        std::map<std::string, BaseSource *> mSources;
        std::vector<Stage *> mStages;

        glm::vec2 mOriginLatLon{};
        glm::vec3 mOriginPosition{};

        KCore::FrustumCulling mCullingFilter{};

        std::map<std::string, GenericTile *> mCreatedBaseTiles{}, mCreatedMetaTiles{};
        std::map<std::string, bool> mCurrBaseTiles{}, mPrevBaseTiles{};
        std::map<std::string, bool> mCurrMetaTiles{}, mPrevMetaTiles{};

        TaskContext mTaskContext{};

#ifndef EMSCRIPTEN
        NetworkContext mNetworkContext{};
#endif

        std::mutex mSyncLock, mAsyncLock;
        std::vector<KCore::MapEvent> mSyncEvents, mAsyncEvents;

    public:
        BaseWorld();

        BaseWorld(float latitude, float longitude);

        std::map<std::string, GenericTile *> &getCreatedBaseTiles();

        std::map<std::string, GenericTile *> &getCreatedMetaTiles();

        [[nodiscard]]
        std::map<std::string, TileDescription> getTiles();

        glm::vec2 latLonToWorldPosition(const glm::vec2 &latLon);

        glm::vec2 worldPositionToLatLon(const glm::vec2 &point);

        void updateFrustum(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix);

        void setPosition(const glm::vec3 &position);

        virtual void update();

        [[nodiscard]]
        const std::map<std::string, BaseSource *> &getSources() const;

        void registerSource(BaseSource *source, const std::string &as);

        Stage *registerStage(Stage *stage);

        std::size_t getSyncEventsLength();

        std::size_t getAsyncEventsLength();

        [[nodiscard]]
        std::vector<KCore::MapEvent> getAsyncEvents();

        [[nodiscard]]
        std::vector<KCore::MapEvent> getSyncEvents();

        void pushToSyncEvents(const MapEvent &event);

        void pushToAsyncEvents(const MapEvent &event);

        std::map<std::string, TileDescription> getCurrentBaseTiles();

        std::map<std::string, TileDescription> getPreviousBaseTiles();

        std::map<std::string, TileDescription> getCurrentMetaTiles();

        std::map<std::string, TileDescription> getPreviousMetaTiles();

        std::map<std::string, BaseSource *> &getSources();

        TaskContext &getTaskContext();

#ifndef __EMSCRIPTEN__
        NetworkContext &getNetworkContext();
#endif

    protected:
        virtual void performStages() = 0;

        virtual void createBaseTileResources(GenericTile *tile) = 0;

        virtual void calculateTiles();

        virtual void postTileCalculation(const std::vector<TileDescription> &tiles) = 0;

        virtual void postMetaTileCalculation() = 0;

        bool screenSpaceError(TileDescription &tile, float quality);

        bool checkTileInFrustum(const TileDescription &tile);

        TileDescription createTile(const std::string &quadcode);
    };
}