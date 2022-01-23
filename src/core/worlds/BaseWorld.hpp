#pragma once

#include <list>
#include <map>

#include "../misc/FrustumCulling.hpp"
#include "../geography/TileDescription.hpp"

#include "../cache/TimeoutCache.hpp"

namespace KCore {
    struct WorldConfig {
        bool GenerateMeta;
    };

    class BaseWorld {
    protected:
        WorldConfig mConfig;

        KCore::FrustumCulling mCullingFilter{};

        TimeoutCache<TileDescription> mTilesCache;
        std::vector<TileDescription> mCommonTiles{};

        glm::vec2 mOriginPositionWGS84{};
        glm::vec3 mOriginPositionOGL{};

    public:
        BaseWorld(const glm::vec2 &originLatLon, const glm::vec2 &originPoint, const struct WorldConfig &config)
                : mOriginPositionWGS84(originPoint), mConfig(config) {}

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
            return projectedPoint - mOriginPositionWGS84;
        }

        glm::vec2 glPointToLatLon(const glm::vec2 &point) {
            auto projectedPoint = point + mOriginPositionWGS84;
            return GeographyConverter::pointToLatLon(projectedPoint);
        }

        void updateFrustum(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix) {
            mCullingFilter.update_frustum(projectionMatrix, viewMatrix);
        }

        void setPosition(const glm::vec3 &position) {
            mOriginPositionOGL = position;
        }

        virtual void update() {
            calculateTiles();
        }

    protected:
        virtual void calculateTiles() {
            // clear up tile tree
//            mTileTree.clear();

            // clear up all mCommonTiles
            mCommonTiles.clear();

            // create height nodes
            for (const auto &item: std::vector{"0", "1", "2", "3"}) {
                auto founded = mTilesCache[item];
                if (founded != std::nullopt) {
                    mCommonTiles.emplace_back(founded.value());
                    continue;
                }

                auto tile = createTile(item);
                auto element = mTilesCache.setOrReplace(item, tile);
                mCommonTiles.push_back(element);
            }

            std::size_t count{0};
            while (count != mCommonTiles.size()) {
                auto tile = mCommonTiles[count];
                auto quadcode = tile.getQuadcode();

                if (screenSpaceError(tile, 3.0)) {
                    if (tile.getType() != TileType::Root)
                        tile.setType(TileType::Separated);
                    tile.setVisibility(TileVisibility::Hide);

                    auto founded = mTilesCache[quadcode + "0"];
                    if (founded != std::nullopt) {
                        mCommonTiles.push_back(founded.value());
                    } else {
                        auto tileNW = createTile(quadcode + "0");
                        auto tileDescription = mTilesCache.setOrReplace(quadcode + "0", tileNW);
                        mCommonTiles.push_back(tileDescription);
                    }

                    founded = mTilesCache[quadcode + "1"];
                    if (founded != std::nullopt) {
                        mCommonTiles.push_back(founded.value());
                    } else {
                        auto tileNE = createTile(quadcode + "1");
                        auto tileDescription = mTilesCache.setOrReplace(quadcode + "1", tileNE);
                        mCommonTiles.push_back(tileDescription);
                    }

                    founded = mTilesCache[quadcode + "2"];
                    if (founded != std::nullopt) {
                        mCommonTiles.push_back(founded.value());
                    } else {
                        auto tileSW = createTile(quadcode + "2");
                        auto tileDescription = mTilesCache.setOrReplace(quadcode + "2", tileSW);
                        mCommonTiles.push_back(tileDescription);
                    }

                    founded = mTilesCache[quadcode + "3"];
                    if (founded != std::nullopt) {
                        mCommonTiles.push_back(founded.value());
                    } else {
                        auto tileSE = createTile(quadcode + "3");
                        auto tileDescription = mTilesCache.setOrReplace(quadcode + "3", tileSE);
                        mCommonTiles.push_back(tileDescription);
                    }
                }

                count++;
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
            auto distance = glm::length(glm::vec3(center.x, 0, center.y) - mOriginPositionOGL);
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

//            tile.setParent(parent);
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

//        TileDescription findOrCreateTile(const std::string &quadcode, const TileDescription *parent) {
//            if (mTileTree.find(quadcode) != mTileTree.end())
//                return *mTileTree[quadcode];
//
//            return createTile(quadcode, parent);
//        }
    };
}