#pragma once

#include <list>
#include <map>

#include "../misc/FrustumCulling.hpp"
#include "../geography/TileDescription.hpp"

namespace KCore {
    struct WorldConfig {
        bool GenerateMeta;
    };

    class BaseWorld {
    protected:
        WorldConfig mConfig;

        KCore::FrustumCulling mCullingFilter{};

        std::list<TileDescription> mCommonTiles{};
        std::map<std::string, TileDescription *> mTileTree;

        glm::vec2 mOriginPositionWGS84{};
        glm::vec3 mOriginPositionOGL{};

    public:
        BaseWorld(const glm::vec2 &originLatLon, const glm::vec2 &originPoint, const struct WorldConfig &config)
                : mOriginPositionWGS84(originPoint), mConfig(config) {}

        void setConfig(const WorldConfig &config) {
            mConfig = config;
        }

        [[nodiscard]]
        const std::list<TileDescription> &getTiles() {
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
            mTileTree.clear();

            // clear up all mCommonTiles
            mCommonTiles.clear();

            // create root nodes
            for (const auto &item: std::vector{"0", "1", "2", "3"})
                mCommonTiles.emplace_back(createTile(item, nullptr));

            std::size_t count{0};
            while (count != mCommonTiles.size()) {
                // move iterator through list
                auto it = mCommonTiles.begin();
                std::advance(it, count);

                // unpack iterator
                auto tile = &(*it);
                auto quadcode = tile->getQuadcode();

                if (screenSpaceError(*tile, 3.0)) {
                    if (tile->getType() != TileType::Root)
                        tile->setType(TileType::Separated);
                    tile->setVisibility(TileVisibility::Hide);

                    auto tileNW = createTile(quadcode + "0", tile);
                    auto tileNE = createTile(quadcode + "1", tile);
                    auto tileSW = createTile(quadcode + "2", tile);
                    auto tileSE = createTile(quadcode + "3", tile);

                    mCommonTiles.emplace_back(tileNW);
                    mCommonTiles.emplace_back(tileNE);
                    mCommonTiles.emplace_back(tileSW);
                    mCommonTiles.emplace_back(tileSE);

                    auto tileNW_ptr = &(tileNW);
                    auto tileNE_ptr = &(tileNE);
                    auto tileSW_ptr = &(tileSW);
                    auto tileSE_ptr = &(tileSE);

                    auto childs = tile->getChilds();

                    childs.InQuad[NorthWest] = tileNW_ptr;
                    childs.InQuad[NorthEast] = tileNE_ptr;
                    childs.InQuad[SouthWest] = tileSW_ptr;
                    childs.InQuad[SouthEast] = tileSE_ptr;

                    childs.Sides[North] = {tileNW_ptr, tileNE_ptr};
                    childs.Sides[South] = {tileSW_ptr, tileSE_ptr};
                    childs.Sides[West] = {tileNW_ptr, tileSW_ptr};
                    childs.Sides[East] = {tileNE_ptr, tileSE_ptr};
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

            auto result = mCullingFilter.test_box(minX, minY, minZ, maxX, maxY, maxZ);
            return result;
        }

        TileDescription createTile(const std::string &quadcode, const TileDescription *parent) {
            TileDescription tile(quadcode);

            tile.setParent(parent);
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

        TileDescription findOrCreateTile(const std::string &quadcode, const TileDescription *parent) {
            if (mTileTree.find(quadcode) != mTileTree.end())
                return *mTileTree[quadcode];

            return createTile(quadcode, parent);
        }
    };
}