#include "Layer.hpp"

#include "../misc/Utils.hpp"

#include "../network/HTTPRequestAdapter/HTTPRequestNetworkAdapter.hpp"
#include "../misc/STBImageUtils.hpp"

namespace KCore {
    Layer::Layer() : Layer(0.0f, 0.0f) {}

    Layer::Layer(float latitude, float longitude) {
        mOriginLatLon = GeographyConverter::latLonToPoint({latitude, longitude});
        mOriginPosition = {latitude, 0.0f, longitude};

        mNetworkAdapter = new HTTPRequestNetworkAdapter();

        // set defaults
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

        processTiles();
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

    std::vector<LayerEvent> Layer::getCoreEventsCopyAndClearQueue() {
        std::lock_guard<std::mutex> lock{mQueueLock};

        std::vector<LayerEvent> copy;
        copy.insert(copy.end(), mCoreEventsQueue.begin(), mCoreEventsQueue.end());

        mCoreEventsQueue.clear();

        return copy;
    }

    std::vector<LayerEvent> Layer::getImageEventsCopyAndClearQueue() {
        std::lock_guard<std::mutex> lock{mQueueLock};

        std::vector<LayerEvent> copy;
        copy.insert(copy.end(), mImageEventsQueue.begin(), mImageEventsQueue.end());

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

    void Layer::processTiles(float target) {
        auto subdivisionResult = subdivideSpace(target);

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
            bool inPrev = mPrevTiles.contains(item);
            bool inNew = mCurrTiles.contains(item);
            if (inNew) {
                auto payload = &mCurrTiles[item].mPayload;
                pushToCoreEvents(LayerEvent::MakeInFrustumEvent(item, payload));
                auto desc = mCurrTiles[item];

                mNetworkAdapter->AsyncGETRequest(
                        mRemoteSource->bakeUrl(desc),
                        [this, desc](const std::vector<uint8_t> &result) {
                            int width = -1, height = -1, channels = -1;
                            auto results = STBImageUtils::decodeImageBuffer(
                                    result.data(), result.size(), width, height, channels
                            );
                            auto image = new ImageResult{width, height, channels, results};
                            pushToImageEvents(LayerEvent::MakeImageEvent(desc.getQuadcode(), image));
                        }
                );
            }
            if (inPrev)
                pushToCoreEvents(LayerEvent::MakeNotInFrustumEvent(item));
        }
    }

    int Layer::test() {
        return 99887766;
    }
}