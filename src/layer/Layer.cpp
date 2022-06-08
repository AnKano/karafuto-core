#include "Layer.hpp"

#include "../misc/Utils.hpp"

//#include "presenters/vulkan/VulkanRenderContext.hpp"
#include "presenters/opencl/OpenCLRenderContext.hpp"
#include "presenters/debug/DebugRenderContext.hpp"
#include "presenters/one-to-one/OneToOneContext.hpp"

#include "../misc/Requests.hpp"

namespace KCore {
    Layer::Layer() : Layer(0.0f, 0.0f) {}

    Layer::Layer(float latitude, float longitude) {
        mOriginLatLon = GeographyConverter::latLonToPoint({latitude, longitude});
        mOriginPosition = {latitude, 0.0f, longitude};

        mRenderContext = new OpenCL::OpenCLRenderContext{this};
//         mRenderContext = new Vulkan::VulkanRenderContext{this};
//        mRenderContext = new OneToOne::OneToOneContext(this);

        // set defaults
        setOneToOneLODMode(1.0f);
        setRasterUrl("http://tile.openstreetmap.org/{z}/{x}/{y}.png");
    }

    void Layer::update() {
        calculateTiles();
    }

    glm::vec2 Layer::latLonToWorldPosition(const glm::vec2 &latLon) {
        auto projectedPoint = GeographyConverter::latLonToPoint(latLon);
        return projectedPoint - mOriginLatLon;
    }

    glm::vec2 Layer::worldPositionToLatLon(const glm::vec2 &point) {
        auto projectedPoint = point + mOriginLatLon;
        return GeographyConverter::pointToLatLon(projectedPoint);
    }

    void Layer::updateFrustum(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix) {
        mCullingFilter.updateFrustum(projectionMatrix, viewMatrix);
    }

    void Layer::setPosition(const glm::vec3 &position) {
        mOriginPosition = position;
    }

    void Layer::calculateTiles() {
        mTiles = {};

        // store old tiles and clear up current
        mPrevTiles = std::move(mCurrTiles);
        mCurrTiles = {};

        mTileProcessor();
    }

    void Layer::pushToCoreEvents(const LayerEvent &event) {
        std::lock_guard<std::mutex> lock{mQueueLock};
        mCoreEventsQueue.push_back(event);
    }

    void Layer::pushToImageEvents(const LayerEvent &event) {
        std::lock_guard<std::mutex> lock{mQueueLock};
        mImageEventsQueue.push_back(event);
    }

    std::size_t Layer::imageEventsCount() {
        std::lock_guard<std::mutex> lock{mQueueLock};
        return mImageEventsQueue.size();
    }

    std::vector<LayerEvent> Layer::getEventsCopyAndClearQueue() {
        std::lock_guard<std::mutex> lock{mQueueLock};

        std::vector<LayerEvent> copy = mCoreEventsQueue;
        copy.insert(copy.end(), mImageEventsQueue.begin(), mImageEventsQueue.end());

        mCoreEventsQueue.clear();
        mImageEventsQueue.clear();

        return copy;
    }

    std::vector<TileDescription> Layer::subdivideSpace(float target) {
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

    TileDescription Layer::createTile(const std::string &quadcode) {
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
            tile.setSideLength(boundsWorld[1] - boundsWorld[3]);
        }

        return tile;
    }

    bool Layer::screenSpaceError(TileDescription &tile, float target, float quality) {
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

    bool Layer::checkTileInFrustum(const TileDescription &tile) {
        auto pos = tile.getCenter();
        auto scale = tile.getSideLength();

        float minX = pos.x - scale / 2.0f;
        float maxX = pos.x + scale / 2.0f;
        float minZ = pos.y - scale / 2.0f;
        float maxZ = pos.y + scale / 2.0f;
        float minY = -1.0f, maxY = 1.0f;

        return mCullingFilter.testAABB(minX, minY, minZ, maxX, maxY, maxZ);
    }

    void Layer::setRasterUrl(const char *url) {
        mRemoteSource = std::make_unique<RemoteSource>(url);
        mRequested.clear();
    }

    void Layer::setOneToOneLODMode(float subdivisionTarget) {
        mTileProcessor = [this, subdivisionTarget]() {
            auto subdivisionResult = subdivideSpace(subdivisionTarget);

            // filter & associate
            for (auto &item: subdivisionResult) {
                if (item.getVisibility() == Visible) {
                    mCurrTiles[item.getQuadcode()] = item;
                    item.setRelatedQuadcodes({item.getQuadcode()});

                    mTiles.push_back(item);
                }
            }

            auto diff = mapKeysDifference<std::string>(mCurrTiles, mPrevTiles);
            auto inter = mapKeysIntersection<std::string>(mCurrTiles, mPrevTiles);

            for (auto &item: diff) {
                bool inPrev = mPrevTiles.count(item) > 0;
                bool inNew = mCurrTiles.count(item) > 0;

                if (inNew) {
                    auto payload = &mCurrTiles[item].mPayload;
                    pushToCoreEvents(LayerEvent::MakeInFrustumEvent(item, payload));
                    auto desc = mCurrTiles[item];

                    if (mRemoteSource == nullptr) continue;
                    performGETRequest(mRemoteSource->bakeUrl(desc), [this, desc](const std::vector<uint8_t> &result) {
                        if (mRenderContext != nullptr)
                            mRenderContext->storeTextureInContext(result, desc.getQuadcode());
                    });
                }

                if (inPrev)
                    pushToCoreEvents(LayerEvent::MakeNotInFrustumEvent(item));
            }

            if (mRenderContext != nullptr)
                mRenderContext->setCurrentTileState(mTiles);
        };
    }

    void Layer::setOneToSubdivisionLODMode(float subdivisionTarget, float additionalSubdivisionTarget) {
        mTileProcessor = [this, subdivisionTarget, additionalSubdivisionTarget]() {
            // scope for results isolation
            {
                auto hldDivision = subdivideSpace(subdivisionTarget);
                auto lldDivision = subdivideSpace(additionalSubdivisionTarget);

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
                    pushToCoreEvents(LayerEvent::MakeInFrustumEvent(item, payload));
                    auto desc = mCurrTiles[item];
                }

                if (inPrev) {
                    pushToCoreEvents(LayerEvent::MakeNotInFrustumEvent(item));
                }
            }

            for (const auto &item: mTiles) {
                for (const auto &related: item.getRelatedQuadcodes()) {
                    if (mRequested.contains(related)) continue;

                    performGETRequest(mRemoteSource->bakeUrl(createTile(related)),
                                      [this, related](const std::vector<uint8_t> &result) {
                                          if (mRenderContext != nullptr)
                                              mRenderContext->storeTextureInContext(result, related);
                                      });

                    mRequested[related] = true;
                }
            }

            if (mRenderContext != nullptr)
                mRenderContext->setCurrentTileState(mTiles);
        };
    }
}