//
// Created by anshu on 6/5/2022.
//

#include "World.hpp"

#include "misc/Utils.hpp"

#include "contexts/network/basic/BasicNetworkContext.hpp"
//#include "contexts/network/debug/DebugNetworkContext.hpp"

#include "contexts/rendering/opencl/OpenCLRenderContext.hpp"
//#include "contexts/rendering/debug/DebugRenderContext.hpp"
//#include "contexts/rendering/one-to-one/OneToOneContext.hpp"

namespace KCore {
    World::World() : World(0.0f, 0.0f) {}

    World::World(float latitude, float longitude) {
        mOriginLatLon = GeographyConverter::latLonToPoint({latitude, longitude});
        mOriginPosition = {latitude, 0.0f, longitude};

        mNetworkContext = new BasicNetworkContext{};
//        mNetworkContext = new DebugNetworkContext{};
        mRemoteSource = new RemoteSource("http://tile.openstreetmap.org/{z}/{x}/{y}.png");

//        auto token = "";
//        auto networkUrl = std::string{"http://api.mapbox.com/v4/mapbox.satellite/{z}/{x}/{y}.png?access_token="} + token;
//        mRemoteSource = new RemoteSource(networkUrl);

        mRenderContext = new OpenCL::OpenCLRenderContext(this);
//        mRenderContext = new OneToOne::OneToOneContext(this);
    }

    void World::update() {
        calculateTiles();
    }

    glm::vec2 World::latLonToWorldPosition(const glm::vec2 &latLon) {
        auto projectedPoint = GeographyConverter::latLonToPoint(latLon);
        return projectedPoint - mOriginLatLon;
    }

    glm::vec2 World::worldPositionToLatLon(const glm::vec2 &point) {
        auto projectedPoint = point + mOriginLatLon;
        return GeographyConverter::pointToLatLon(projectedPoint);
    }

    void World::updateFrustum(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix) {
        mCullingFilter.updateFrustum(projectionMatrix, viewMatrix);
    }

    void World::setPosition(const glm::vec3 &position) {
        mOriginPosition = position;
    }

    void World::calculateTiles() {
        mTiles = {};

        // store old tiles and clear up current
        mPrevTiles = std::move(mCurrTiles);
        mCurrTiles = {};

        {
            auto hldDivision = subdivideSpace(1.0);
            auto lldDivision = subdivideSpace(2.5);

            for (auto &ll: lldDivision) {
                if (ll.getVisibility() != TileVisibility::Visible) continue;
                const auto &llQuadcode = ll.getQuadcode();

                std::vector<std::string> childs{};

                for (const auto &hl: hldDivision) {
                    if (hl.getVisibility() != TileVisibility::Visible) continue;
                    const auto &hlQuadcode = hl.getQuadcode();

                    if (hlQuadcode == llQuadcode) continue;
                    if (hlQuadcode.starts_with(llQuadcode))
                        childs.push_back(hlQuadcode);
                }

                if (childs.empty())
                    childs.push_back(llQuadcode);

                ll.setRelatedQuadcodes(childs);
                mCurrTiles[llQuadcode] = ll;
                mTiles.push_back(ll);
            }
        }

        auto diff = mapKeysDifference<std::string>(mCurrTiles, mPrevTiles);
        auto inter = mapKeysIntersection<std::string>(mCurrTiles, mPrevTiles);

        for (auto &item: diff) {
            bool inPrev = mPrevTiles.contains(item);
            bool inNew = mCurrTiles.contains(item);

            if (inNew) {
                auto payload = &mCurrTiles[item].mPayload;

                auto event = Event::MakeInFrustumEvent(item, payload);
                pushToCoreEvents(event);

                auto desc = mCurrTiles[item];

                if (mRemoteSource == nullptr) continue;
                if (mNetworkContext == nullptr) continue;
            }

            if (inPrev) {
                auto event = Event::MakeNotInFrustumEvent(item);
                pushToCoreEvents(event);
            }
        }

        for (const auto &item: mTiles) {
            // !TODO: stop load this shit every fucking time!!!
            for (const auto &related: item.getRelatedQuadcodes()) {
                if (mRequested.contains(related)) continue;

                mNetworkContext->pushRequestToQueue(
                        new KCore::NetworkRequest{
                                mRemoteSource->bakeUrl(createTile(related)),
                                [this, related](const std::vector<uint8_t> &data) {
                                    if (mRenderContext != nullptr)
                                        mRenderContext->storeTextureInContext(data, related);
                                }, nullptr
                        }
                );

                mRequested[related] = true;
            }
        }

        if (mRenderContext != nullptr)
            mRenderContext->setCurrentTileState(mTiles);

//        auto subdivisionResult = subdivideSpace(1.0);
//
//        // filter & associate
//        for (auto &item: subdivisionResult) {
//            if (item.getVisibility() == Visible) {
//                mCurrTiles[item.getQuadcode()] = item;
//                item.setRelatedQuadcodes({item.getQuadcode()});
//
//                mTiles.push_back(item);
//            }
//        }
//
//        auto diff = mapKeysDifference<std::string>(mCurrTiles, mPrevTiles);
//        auto inter = mapKeysIntersection<std::string>(mCurrTiles, mPrevTiles);
//
//        for (auto &item: diff) {
//            bool inPrev = mPrevTiles.count(item) > 0;
//            bool inNew = mCurrTiles.count(item) > 0;
//
//            if (inNew) {
//                auto payload = &mCurrTiles[item].mPayload;
//
//                auto event = Event::MakeInFrustumEvent(item, payload);
//                pushToCoreEvents(event);
//
//                auto desc = mCurrTiles[item];
//
//                if (mRemoteSource == nullptr) continue;
//                if (mNetworkContext == nullptr) continue;
//
//                auto url = mRemoteSource->bakeUrl(desc);
//                auto request = new KCore::NetworkRequest{
//                        url, [this, desc](const std::vector<uint8_t> &data) {
////                            std::cout << "something was downloaded to " << desc.getQuadcode()  << std::endl;
//                            if (mRenderContext != nullptr)
//                                mRenderContext->storeTextureInContext(data, desc.getQuadcode());
//                        }, nullptr
//                };
//                mNetworkContext->pushRequestToQueue(request);
//            }
//
//            if (inPrev) {
//                auto event = Event::MakeNotInFrustumEvent(item);
//                pushToCoreEvents(event);
//            }
//        }
//
//        if (mRenderContext != nullptr)
//            mRenderContext->setCurrentTileState(mTiles);
    }

    void World::pushToCoreEvents(const Event &event) {
        std::lock_guard<std::mutex> lock{mQueueLock};
        mCoreEventsQueue.push_back(event);
    }

    void World::pushToImageEvents(const Event &event) {
        std::lock_guard<std::mutex> lock{mQueueLock};
        mImageEventsQueue.push_back(event);
    }

    std::size_t World::imageEventsCount() {
        std::lock_guard<std::mutex> lock{mQueueLock};
        return mImageEventsQueue.size();
    }

    std::vector<Event> World::getEventsCopyAndClearQueue() {
        std::lock_guard<std::mutex> lock{mQueueLock};

        std::vector<Event> copy = mCoreEventsQueue;
        copy.insert(copy.end(), mImageEventsQueue.begin(), mImageEventsQueue.end());

        mCoreEventsQueue.clear();
        mImageEventsQueue.clear();

        return copy;
    }

    std::vector<TileDescription> World::subdivideSpace(float target) {
        std::vector<TileDescription> tiles{};

        // create tiles
        for (const auto &item: std::vector{"0", "1", "2", "3"})
            tiles.push_back(createTile(item));

        std::size_t count = 0;
        while (count != tiles.size()) {
            auto tile = &tiles[count];
            auto quadcode = tile->getQuadcode();

            if (screenSpaceError(*tile, target)) {
                if (tile->getType() != TileType::Root) tile->setType(TileType::Separated);
                tile->setVisibility(TileVisibility::Hide);

                for (const auto &item: std::vector{"0", "1", "2", "3"})
                    tiles.push_back(createTile(quadcode + item));
            }

            count++;
        }

        return tiles;
    }

    TileDescription World::createTile(const std::string &quadcode) {
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

    bool World::screenSpaceError(TileDescription &tile, float target, float quality) {
        const auto &quadcode = tile.getQuadcode();

        if (!checkTileInFrustum(tile)) {
            // if it's not in frustum just hide it without separation
            tile.setVisibility(TileVisibility::Hide);
            return false;
        }

        auto center = tile.getCenter();
        auto distance = glm::length(glm::vec3(center.x, 0, center.y) - mOriginPosition);
        auto error = quality * tile.getSideLength() / distance;

        return error > target;
    }

    bool World::checkTileInFrustum(const TileDescription &tile) {
        auto pos = tile.getCenter();
        auto scale = tile.getSideLength();

        float half = scale / 2.0f;

        float minX = (float) pos.x - half, maxX = (float) pos.x + half;
        float minZ = (float) pos.y - half, maxZ = (float) pos.y + half;
        float minY = -1.0f, maxY = 1.0f;

        auto result = mCullingFilter.testAABB(minX, minY, minZ, maxX, maxY, maxZ);
        return result;
    }

    INetworkContext *World::getNetworkContext() {
        return mNetworkContext;
    }

    IRenderContext *World::getRenderContext() {
        return mRenderContext;
    }
}