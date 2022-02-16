#include "PlainWorld.hpp"
#include "../misc/STBImageUtils.hpp"

namespace KCore {
    void PlainWorld::calculateTiles() {
        BaseWorld::calculateTiles();
    }

    void PlainWorld::makeEvents() {
        mSyncEvents.clear();

        for (const auto &item: mStages)
            item->invoke(this);
    }

    extern "C" {
    DllExport KCore::PlainWorld *CreatePlainWorld(float latitude, float longitude) {
        return new KCore::PlainWorld(latitude, longitude);
    }
    }
}