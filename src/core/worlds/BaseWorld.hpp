#pragma once

#include <list>
#include <map>

#include "../misc/FrustumCulling.hpp"
#include "../geography/TileDescription.hpp"
#include "../cache/TimeoutCache.hpp"

namespace KCore {
    class MapCore;

    struct WorldConfig {
        bool GenerateMeta;
    };

    class BaseWorld {
    protected:
        MapCore *core;

        WorldConfig mConfig;

        KCore::FrustumCulling mCullingFilter{};

        TimeoutCache<TileDescription> mTilesCache;
        std::vector<TileDescription> mCommonTiles{};

        glm::vec2 mOriginPositionReal{};
        glm::vec3 mOriginPositionWebMercator{};

    public:
        BaseWorld(const glm::vec2 &originLatLon, const glm::vec2 &originPoint,
                  const struct WorldConfig &config)
                : mOriginPositionReal(originPoint),
                  mOriginPositionWebMercator(originPoint.x, 0.0f, originPoint.y),
                  mConfig(config) {}

        void setConfig(const WorldConfig &config) {
            mConfig = config;
        }

        [[nodiscard]]
        const std::vector<TileDescription> &getTiles() {
            return mCommonTiles;
        }

        [[nodiscard]]
        const WorldConfig &getConfig() const {
            return mConfig;
        }

        glm::vec2 latLonToGlPoint(const glm::vec2 &latLon) {
            auto projectedPoint = GeographyConverter::latLonToPoint(latLon);
            return projectedPoint - mOriginPositionReal;
        }

        glm::vec2 glPointToLatLon(const glm::vec2 &point) {
            auto projectedPoint = point + mOriginPositionReal;
            return GeographyConverter::pointToLatLon(projectedPoint);
        }

        void updateFrustum(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix) {
            mCullingFilter.update_frustum(projectionMatrix, viewMatrix);
        }

        void setPosition(const glm::vec3 &position) {
            mOriginPositionWebMercator = position;
        }

        virtual void update() {
            calculateTiles();
        }

    protected:
        virtual void calculateTiles() {
            // clear up all mCommonTiles
            mCommonTiles.clear();

            std::vector<TileDescription> tiles{};

            // create tiles
            for (const auto &item: std::vector{"0", "1", "2", "3"}) {
                auto founded = mTilesCache[item];
                if (founded) {
                    tiles.emplace_back(*founded);
                    continue;
                }

                auto tile = createTile(item);
                auto element = mTilesCache.setOrReplace(item, tile);
                tiles.push_back(element);
            }

            std::size_t count{0};
            while (count != tiles.size()) {
                auto tile = &tiles[count];
                auto quadcode = tile->getQuadcode();

                if (screenSpaceError(*tile, 3.0)) {
                    if (tile->getType() != TileType::Root)
                        tile->setType(TileType::Separated);
                    tile->setVisibility(TileVisibility::Hide);

                    for (const auto &item: std::vector{"0", "1", "2", "3"}) {
                        auto founded = mTilesCache[quadcode + item];
                        if (founded) {
                            tiles.emplace_back(*founded);
                        } else {
                            auto child = createTile(quadcode + item);
                            auto tileDescription = mTilesCache.setOrReplace(quadcode + item, child);
                            tiles.push_back(tileDescription);
                        }
                    }
                }

                count++;
            }

            auto condition = [this](const TileDescription &tile) {
                if (tile.getVisibility() != Visible) return false;
                const auto center = tile.getCenter();
                auto distance = glm::length(glm::vec3(center.x, 0, center.y) - mOriginPositionWebMercator);
                return distance <= 500000.0f;
            };

            std::copy_if(tiles.begin(), tiles.end(), std::back_inserter(mCommonTiles), condition);
        }

        bool screenSpaceError(TileDescription &tile, float quality) {
            const auto &quadcode = tile.getQuadcode();

            if (!checkTileInFrustum(tile)) {
                // if it's not in frustum just hide it without separation
                tile.setVisibility(TileVisibility::Hide);
                return false;
            }

            auto center = tile.getCenter();
            auto distance = glm::length(glm::vec3(center.x, 0, center.y) - mOriginPositionWebMercator);
            auto error = quality * tile.getSideLength() / distance;

            return error > 1.0f;
        }

        bool checkTileInFrustum(const TileDescription &tile) {
            auto pos = tile.getCenter();
            auto scale = tile.getSideLength();

            float minX = (float) pos.x - scale, maxX = (float) pos.x + scale;
            float minZ = (float) pos.y - scale, maxZ = (float) pos.y + scale;
            float minY = 0.0f, maxY = 0.0f;

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

                auto sw = latLonToGlPoint({boundsLatLon[1], boundsLatLon[0]});
                auto ne = latLonToGlPoint({boundsLatLon[3], boundsLatLon[2]});

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
                tile.setCenterLatLon(glPointToLatLon(tile.getCenter()));
            }

            return tile;
        }
    };
}