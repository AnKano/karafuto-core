#include "PlainWorld.hpp"

namespace KCore {
    void PlainWorld::calculateTiles() {
        BaseWorld::calculateTiles();
    }

    void PlainWorld::makeEvents() {
        mSyncEvents.clear();

        auto diff = mapKeysDifference<std::string>(mPrevBaseTiles, mCurrBaseTiles);
        auto inter = mapKeysIntersection<std::string>(mPrevBaseTiles, mCurrBaseTiles);

        for (auto &item: diff) {
            bool inPrev = mPrevBaseTiles.count(item) > 0;
            bool inNew = mCurrBaseTiles.count(item) > 0;

            if (inPrev) {
                mSyncEvents.push_back(MapEvent::MakeNotInFrustumEvent(item));
            }

            if (inNew) {
                mSyncEvents.push_back(MapEvent::MakeInFrustumEvent(
                        item, &mCurrBaseTiles[item].mPayload
                ));
            }
        }
    }

    extern "C" {
    DllExport KCore::PlainWorld *CreatePlainWorld(float latitude, float longitude) {
        return new KCore::PlainWorld(latitude, longitude);
    }
    }
}