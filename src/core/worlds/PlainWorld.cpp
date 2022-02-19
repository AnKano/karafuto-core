#include "PlainWorld.hpp"

namespace KCore {
    PlainWorld::PlainWorld() : BaseWorld(0.0f, 0.0f) {}

    PlainWorld::PlainWorld(float latitude, float longitude) : BaseWorld(latitude, longitude) {
        registerStage(KCore::BuiltInStages::CommonCalculate());
    }

    void PlainWorld::calculateTiles() {
        BaseWorld::calculateTiles();
    }

    void PlainWorld::performStages() {
        mSyncEvents.clear();

        for (const auto &item: mStages)
            item->invoke(this);

        mIteration++;
    }

    void PlainWorld::createBaseTileResources(GenericTile *tile) {
        tile->registerImmediateResource("image", BuiltInResource::ImageCalculate());
        tile->registerImmediateResource("json", BuiltInResource::JSONCalculate());
    }

    void PlainWorld::postTileCalculation(const std::vector<TileDescription> &tiles) {
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
                    createBaseTileResources(mCreatedBaseTiles[quadcode]);
                    mCreatedBaseTiles[quadcode]->invokeResources();
                }

                mCurrBaseTiles[quadcode] = true;
            }
        }
    }

    void PlainWorld::postMetaTileCalculation() {}

    extern "C" {
    DllExport KCore::PlainWorld *CreatePlainWorld(float latitude, float longitude) {
        return new KCore::PlainWorld(latitude, longitude);
    }
    }
}