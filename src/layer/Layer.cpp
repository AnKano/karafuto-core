#include "Layer.hpp"

#include "../misc/Utils.hpp"
#include "../network/HTTPRequestAdapter/HTTPRequestNetworkAdapter.hpp"

namespace KCore {
    Layer::Layer() : Layer(0.0f, 0.0f) {}

    Layer::Layer
            (float latitude, float longitude) {
        mOriginLatLon = GeographyConverter::latLonToPoint({latitude, longitude});
        mOriginPosition = {latitude, 0.0f, longitude};

        mNetworkAdapter = new HTTPRequestNetworkAdapter();

        // set defaults
        setRasterUrl("http://tile.openstreetmap.org/{z}/{x}/{y}.png");
    }

    void Layer::update() {
        calculateTiles();
    }

    glm::vec2 Layer::latLonToWorldPosition
            (const glm::vec2 &latLon) const {
        auto projectedPoint = GeographyConverter::latLonToPoint(latLon);
        return projectedPoint - mOriginLatLon;
    }

    glm::vec2 Layer::worldPositionToLatLon
            (const glm::vec2 &point) const {
        auto projectedPoint = point + mOriginLatLon;
        return GeographyConverter::pointToLatLon(projectedPoint);
    }

    void Layer::updateFrustum
            (const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix) {
        mCullingFilter.updateFrustum(projectionMatrix, viewMatrix);
    }

    void Layer::setPosition
            (const glm::vec3 &position) {
        mOriginPosition = position;
    }

    void Layer::calculateTiles() {
        mTiles = {};

        // store old tiles and clear up current
        mPrevTiles = std::move(mCurrTiles);
        mCurrTiles = {};

        processTiles();
    }

    void Layer::pushToCoreEvents
            (const LayerEvent &event) {
        std::lock_guard<std::mutex> lock{mQueueLock};
        mCoreEventsQueue.push_back(event);
    }

    void Layer::pushToImageEvents
            (const LayerEvent &event) {
        std::lock_guard<std::mutex> lock{mQueueLock};
        mImageEventsQueue.push_back(event);
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

    std::vector<TileDescription> Layer::subdivideSpace
            (float target) {
        std::vector<TileDescription> tiles{};

        // create tiles
        for (const auto &item: std::vector{"0", "1", "2", "3"})
            tiles.emplace_back(this, item);

        std::size_t count = 0;
        while (count != tiles.size()) {
            auto tile = &tiles[count];
            auto quadcode = tile->getQuadcode();

            if (screenSpaceError(*tile, target)) {
                if (tile->getType() != TileType::Root) tile->setType(TileType::Separated);
                tile->setVisibility(TileVisibility::Hide);

                for (const auto &item: std::vector{"0", "1", "2", "3"})
                    tiles.emplace_back(this, quadcode + item);
            }

            count++;
        }

        return tiles;
    }

    bool Layer::screenSpaceError
            (TileDescription &tile, float target, float quality) {
        const auto &quadcode = tile.getQuadcode();

        if (!checkTileInFrustum(tile)) {
            // if it's not in frustum just hide it without separation
            tile.setVisibility(TileVisibility::Hide);
            return false;
        }

        auto center = tile.getCenter();
        auto distance = glm::length(glm::vec3(center.x, 0, center.y) - mOriginPosition);
        auto error = quality * tile.getScale() / distance;

        return error > target;
    }

    bool Layer::checkTileInFrustum
            (const TileDescription &tile) {
        auto pos = tile.getCenter();
        auto scale = tile.getScale();

        float minX = pos.x - scale / 2.0f;
        float maxX = pos.x + scale / 2.0f;
        float minZ = pos.y - scale / 2.0f;
        float maxZ = pos.y + scale / 2.0f;
        float minY = -1.0f, maxY = 1.0f;

        return mCullingFilter.testAABB(minX, minY, minZ, maxX, maxY, maxZ);
    }

    void Layer::setRasterUrl
            (const char *url) {
        mRemoteSource = std::make_unique<RemoteSource>(url);
        mRequested.clear();
    }

    void Layer::processTiles
            (float target) {
        auto subdivisionResult = subdivideSpace(target);

        // filter & associate
        for (auto &item: subdivisionResult) {
            if (item.getVisibility() == Visible) {
                mCurrTiles[item.getQuadcode()] = item;
                mTiles.push_back(item);
            }
        }

        auto diff = mapKeysDifference<std::string>(mCurrTiles, mPrevTiles);
        auto inter = mapKeysIntersection<std::string>(mCurrTiles, mPrevTiles);

        for (auto &quadcode: diff) {
            bool inPrev = mPrevTiles.contains(quadcode);
            bool inNew = mCurrTiles.contains(quadcode);

            if (inNew) {
                auto desc = mCurrTiles[quadcode];

                pushToCoreEvents(LayerEvent::MakeInFrustumEvent(quadcode, mCurrTiles[quadcode]));

                mNetworkAdapter->AsyncGETRequest(
                        mRemoteSource->bakeUrl(desc),
                        [this, quadcode](const std::vector<uint8_t> &result) {
                            pushToImageEvents(LayerEvent::MakeImageEvent(quadcode, result));
                        }
                );
            }

            if (inPrev)
                pushToCoreEvents(LayerEvent::MakeNotInFrustumEvent(quadcode));
        }
    }
}