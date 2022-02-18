#pragma once

#include <unordered_map>
#include <map>

#include "../misc/FrustumCulling.hpp"
#include "../geography/TileDescription.hpp"
#include "../cache/TimeoutCache.hpp"
#include "../contexts/task/TaskContext.hpp"
#include "../contexts/network/NetworkContext.hpp"
#include "../sources/BaseSource.hpp"
#include "../geography/tiles/GenericTile.hpp"
#include "stages/Stage.hpp"

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
        NetworkContext mNetworkContext{};

        std::mutex mSyncLock, mAsyncLock;
        std::vector<KCore::MapEvent> mSyncEvents, mAsyncEvents;

    public:
        BaseWorld() : BaseWorld(0.0f, 0.0f) {}

        BaseWorld(float latitude, float longitude) {
            glm::vec2 originPoint{GeographyConverter::latLonToPoint({latitude, longitude})};
            mOriginLatLon = originPoint;
            mOriginPosition = {latitude, 0.0f, longitude};
        }

        std::map<std::string, GenericTile*> &getCreatedTiles() {
            return mCreatedBaseTiles;
        }

        [[nodiscard]]
        std::map<std::string, TileDescription> getTiles() {
            std::map<std::string, TileDescription> currentTilesCopy{};

            for (const auto &[key, _]: mCurrBaseTiles) {
                currentTilesCopy[key] = mCreatedBaseTiles[key]->getTileDescription();
            }

            return currentTilesCopy;
        }

        glm::vec2 latLonToWorldPosition(const glm::vec2 &latLon) {
            auto projectedPoint = GeographyConverter::latLonToPoint(latLon);
            return projectedPoint - mOriginLatLon;
        }

        glm::vec2 worldPositionToLatLon(const glm::vec2 &point) {
            auto projectedPoint = point + mOriginLatLon;
            return GeographyConverter::pointToLatLon(projectedPoint);
        }

        void updateFrustum(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix) {
            mCullingFilter.update_frustum(projectionMatrix, viewMatrix);
        }

        void setPosition(const glm::vec3 &position) {
            mOriginPosition = position;
        }

        virtual void update() {
            calculateTiles();
            makeEvents();
        }

        void registerSource(BaseSource *source, const std::string &as) {
            mSources[as] = source;
        }

        Stage *registerStage(Stage *stage) {
            mStages.push_back(stage);
            return stage;
        }

        [[nodiscard]]
        std::vector<KCore::MapEvent> getSyncEvents() {
            // unnecessary
            std::lock_guard<std::mutex> lock{mSyncLock};
            // make copy of events
            return mSyncEvents;
        }

        [[nodiscard]]
        std::vector<KCore::MapEvent> getAsyncEvents() {
            std::lock_guard<std::mutex> lock{mAsyncLock};
            // make copy of events
            auto copy = mAsyncEvents;
            // clear all async events
            mAsyncEvents.clear();

            return copy;
        }

        void pushToSyncEvents(const MapEvent &event) {
            std::lock_guard<std::mutex> lock{mSyncLock};
            mSyncEvents.push_back(event);
        }

        void pushToAsyncEvents(const MapEvent &event) {
            std::lock_guard<std::mutex> lock{mAsyncLock};
            mAsyncEvents.push_back(event);
        }

        std::map<std::string, TileDescription> getCurrentBaseTiles() {
            std::map<std::string, TileDescription> currentTilesCopy{};

            for (const auto &[key, _]: mCurrBaseTiles) {
                currentTilesCopy[key] = mCreatedBaseTiles[key]->getTileDescription();
            }

            return currentTilesCopy;
        }

        std::map<std::string, TileDescription> getPreviousBaseTiles() {
            std::map<std::string, TileDescription> prevTilesCopy{};

            for (const auto &[key, _]: mPrevBaseTiles) {
                prevTilesCopy[key] = mCreatedBaseTiles[key]->getTileDescription();
            }

            return prevTilesCopy;
        }

        std::map<std::string, BaseSource *> &getSources() {
            return mSources;
        }

        TaskContext &getTaskContext() {
            return mTaskContext;
        }

        NetworkContext &getNetworkContext() {
            return mNetworkContext;
        }

    protected:
        virtual void makeEvents() = 0;

        virtual void calculateTiles() {
            // store old tiles and clear up current
            mPrevBaseTiles = std::move(mCurrBaseTiles);
            mCurrBaseTiles = {};

            std::vector<TileDescription> tiles{};

            // create tiles
            for (const auto &item: std::vector{"0", "1", "2", "3"})
                tiles.push_back(createTile(item));

            std::size_t count{0};
            while (count != tiles.size()) {
                auto tile = &tiles[count];
                auto quadcode = tile->getQuadcode();

                if (screenSpaceError(*tile, 3.0)) {
                    if (tile->getType() != TileType::Root)
                        tile->setType(TileType::Separated);
                    tile->setVisibility(TileVisibility::Hide);

                    for (const auto &item: std::vector{"0", "1", "2", "3"})
                        tiles.push_back(createTile(quadcode + item));
                }

                count++;
            }

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
                    }

                    mCurrBaseTiles[quadcode] = true;
                }
            }
        }

        bool screenSpaceError(TileDescription &tile, float quality) {
            const auto &quadcode = tile.getQuadcode();

            if (!checkTileInFrustum(tile)) {
                // if it's not in frustum just hide it without separation
                tile.setVisibility(TileVisibility::Hide);
                return false;
            }

            auto center = tile.getCenter();
            auto distance = glm::length(glm::vec3(center.x, 0, center.y) - mOriginPosition);
            auto error = quality * tile.getSideLength() / distance;

            return error > 1.0f;
        }

        bool checkTileInFrustum(const TileDescription &tile) {
            auto pos = tile.getCenter();
            auto scale = tile.getSideLength();

            float minX = (float) pos.x - scale, maxX = (float) pos.x + scale;
            float minZ = (float) pos.y - scale, maxZ = (float) pos.y + scale;
            float minY = -1.0f, maxY = 1.0f;

            auto result = mCullingFilter.testAABB(minX, minY, minZ, maxX, maxY, maxZ);
            return result;
        }

        TileDescription createTile(const std::string &quadcode) {
            TileDescription tile(quadcode);

            tile.setTilecode(GeographyConverter::quadcodeToTilecode(quadcode));

            {
                auto tilecode = tile.getTilecode();

                auto w = GeographyConverter::tileToLon(tilecode[0], tilecode[2]);
                auto s = GeographyConverter::tileToLat(tilecode[1] + 1, tilecode[2]);
                auto e = GeographyConverter::tileToLon(tilecode[0] + 1, tilecode[2]);
                auto n = GeographyConverter::tileToLat(tilecode[1], tilecode[2]);

                tile.setBoundsLatLon({w, s, e, n});
            }

            {
                auto boundsLatLon = tile.getBoundsLatLon();

                auto sw = latLonToWorldPosition({boundsLatLon[1], boundsLatLon[0]});
                auto ne = latLonToWorldPosition({boundsLatLon[3], boundsLatLon[2]});

                tile.setBoundsWorld({sw.x, sw.y, ne.x, ne.y});
            }

            {
                auto boundsWorld = tile.getBoundsWorld();
                auto x = boundsWorld[0] + (boundsWorld[2] - boundsWorld[0]) / 2;
                auto y = boundsWorld[1] + (boundsWorld[3] - boundsWorld[1]) / 2;

                tile.setCenter({x, y});

                auto firstPoint = glm::vec3(boundsWorld[0], 0, boundsWorld[3]);
                auto secondPoint = glm::vec3(boundsWorld[0], 0, boundsWorld[1]);

                tile.setSideLength(glm::length(firstPoint - secondPoint));
                tile.setCenterLatLon(worldPositionToLatLon(tile.getCenter()));
            }

            return tile;
        }
    };
}