#include "BaseWorld.hpp"

#include "../contexts/network/basic/BasicNetworkContext.hpp"
//    #include "../contexts/network/fallback/FallbackNetworkContext.hpp"

namespace KCore {
    BaseWorld::BaseWorld() : BaseWorld(0.0f, 0.0f) {}

    BaseWorld::BaseWorld(float latitude, float longitude) {
        glm::vec2 originPoint{GeographyConverter::latLonToPoint({latitude, longitude})};
        mOriginLatLon = originPoint;
        mOriginPosition = {latitude, 0.0f, longitude};

        mNetworkContext = new BasicNetworkContext{};
//        mNetworkContext = new FallbackNetworkContext{};
    }

    std::map<std::string, GenericTile *> &BaseWorld::getCreatedBaseTiles() {
        return mCreatedBaseTiles;
    }

    std::map<std::string, GenericTile *> &BaseWorld::getCreatedMetaTiles() {
        return mCreatedMetaTiles;
    }

    std::map<std::string, TileDescription> BaseWorld::getTiles() {
        std::map<std::string, TileDescription> currentTilesCopy{};

        for (const auto &[key, _]: mCurrBaseTiles) {
            currentTilesCopy[key] = mCreatedBaseTiles[key]->getTileDescription();
        }

        return currentTilesCopy;
    }

    glm::vec2 BaseWorld::latLonToWorldPosition(const glm::vec2 &latLon) {
        auto projectedPoint = GeographyConverter::latLonToPoint(latLon);
        return projectedPoint - mOriginLatLon;
    }

    glm::vec2 BaseWorld::worldPositionToLatLon(const glm::vec2 &point) {
        auto projectedPoint = point + mOriginLatLon;
        return GeographyConverter::pointToLatLon(projectedPoint);
    }

    void BaseWorld::updateFrustum(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix) {
        mCullingFilter.update_frustum(projectionMatrix, viewMatrix);
    }

    void BaseWorld::setPosition(const glm::vec3 &position) {
        mOriginPosition = position;
    }

    void BaseWorld::update() {
        calculateTiles();
        performStages();
    }

    const std::map<std::string, BaseSource *> &BaseWorld::getSources() const {
        return mSources;
    }

    void BaseWorld::registerSource(BaseSource *source, const std::string &as) {
        mSources[as] = source;
    }

    Stage *BaseWorld::registerStage(Stage *stage) {
        mStages.push_back(stage);
        return stage;
    }

    std::vector<KCore::MapEvent> BaseWorld::getSyncEvents() {
        // unnecessary
        std::lock_guard<std::mutex> lock{mSyncLock};
        // make copy of events
        return mSyncEvents;
    }

    std::vector<KCore::MapEvent> BaseWorld::getAsyncEvents() {
        std::lock_guard<std::mutex> lock{mAsyncLock};
        // make copy of events
        auto copy = mAsyncEvents;
        // clear all async events
        mAsyncEvents.clear();

        return copy;
    }

    void BaseWorld::pushToSyncEvents(const MapEvent &event) {
        std::lock_guard<std::mutex> lock{mSyncLock};
        mSyncEvents.push_back(event);
    }

    void BaseWorld::pushToAsyncEvents(const MapEvent &event) {
        std::lock_guard<std::mutex> lock{mAsyncLock};
        mAsyncEvents.push_back(event);
    }

    std::size_t BaseWorld::getSyncEventsLength() {
        std::lock_guard<std::mutex> lock{mSyncLock};
        return mSyncEvents.size();
    }

    std::size_t BaseWorld::getAsyncEventsLength() {
        std::lock_guard<std::mutex> lock{mAsyncLock};
        return mAsyncEvents.size();
    }

    std::map<std::string, TileDescription> BaseWorld::getCurrentBaseTiles() {
        std::map<std::string, TileDescription> currentTilesCopy{};

        for (const auto &[key, _]: mCurrBaseTiles) {
            currentTilesCopy[key] = mCreatedBaseTiles[key]->getTileDescription();
        }

        return currentTilesCopy;
    }

    std::map<std::string, TileDescription> BaseWorld::getPreviousBaseTiles() {
        std::map<std::string, TileDescription> prevTilesCopy{};

        for (const auto &[key, _]: mPrevBaseTiles) {
            prevTilesCopy[key] = mCreatedBaseTiles[key]->getTileDescription();
        }

        return prevTilesCopy;
    }

    std::map<std::string, TileDescription> BaseWorld::getCurrentMetaTiles() {
        std::map<std::string, TileDescription> currentTilesCopy{};

        for (const auto &[key, _]: mCurrMetaTiles) {
            currentTilesCopy[key] = mCreatedMetaTiles[key]->getTileDescription();
        }

        return currentTilesCopy;
    }

    std::map<std::string, TileDescription> BaseWorld::getPreviousMetaTiles() {
        std::map<std::string, TileDescription> prevTilesCopy{};

        for (const auto &[key, _]: mPrevMetaTiles) {
            prevTilesCopy[key] = mCreatedMetaTiles[key]->getTileDescription();
        }

        return prevTilesCopy;
    }

    std::map<std::string, BaseSource *> &BaseWorld::getSources() {
        return mSources;
    }

    TaskContext &BaseWorld::getTaskContext() {
        return mTaskContext;
    }

    INetworkContext *BaseWorld::getNetworkContext() {
        return mNetworkContext;
    }

    void BaseWorld::calculateTiles() {
        // store old tiles and clear up current
        mPrevBaseTiles = std::move(mCurrBaseTiles);
        mCurrBaseTiles = {};

//        typedef std::chrono::high_resolution_clock Time;
//        typedef std::chrono::milliseconds ms;
//        typedef std::chrono::duration<float> fsec;
//        auto t0 = Time::now();

        std::vector<TileDescription> tiles = divide();

//        auto t1 = Time::now();
//        fsec fs = t1 - t0;
//        ms d = std::chrono::duration_cast<ms>(fs);
//        std::cout << "base step: " << d.count() << "ms\n";

        postTileCalculation(tiles);
    }

    bool BaseWorld::screenSpaceError(TileDescription &tile, float target, float quality) {
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

    bool BaseWorld::checkTileInFrustum(const TileDescription &tile) {
        auto pos = tile.getCenter();
        auto scale = tile.getSideLength();

        float minX = (float) pos.x - scale / 2, maxX = (float) pos.x + scale / 2;
        float minZ = (float) pos.y - scale / 2, maxZ = (float) pos.y + scale / 2;
        float minY = -1.0f, maxY = 1.0f;

        auto result = mCullingFilter.testAABB(minX, minY, minZ, maxX, maxY, maxZ);
        return result;
    }

    TileDescription BaseWorld::createTile(const std::string &quadcode) {
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

    std::vector<TileDescription> BaseWorld::divide(float target) {
        std::vector<TileDescription> tiles{};

        // create tiles
        for (const auto &item: std::vector{"0", "1", "2", "3"})
            tiles.push_back(createTile(item));

        std::size_t count{0};
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
}