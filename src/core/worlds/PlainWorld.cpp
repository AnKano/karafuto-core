#include "PlainWorld.hpp"

namespace KCore {
    PlainWorld::PlainWorld() : BaseWorld(0.0f, 0.0f) {}

    PlainWorld::PlainWorld(float latitude, float longitude) : BaseWorld(latitude, longitude) {
        registerStage(KCore::BuiltInStages::CommonCalculate());
    }

    void PlainWorld::calculateTiles() {
        BaseWorld::calculateTiles();
    }

    void PlainWorld::makeEvents() {
        mSyncEvents.clear();

        for (const auto &item: mStages)
            item->invoke(this);

        mIteration++;
    }

    void PlainWorld::createTileResources(GenericTile *tile) {
        tile->registerImmediateResource("image", BuiltInResource::ImageCalculate());
        tile->registerImmediateResource("json", BuiltInResource::JSONCalculate());
    }

    extern "C" {
    DllExport KCore::PlainWorld *CreatePlainWorld(float latitude, float longitude) {
        return new KCore::PlainWorld(latitude, longitude);
    }
    }
}