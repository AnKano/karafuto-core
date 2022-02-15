#include "PlainWorld.hpp"
#include "../misc/STBImageUtils.hpp"

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
                auto event = MapEvent::MakeNotInFrustumEvent(item);
                pushToSyncEvents(event);
            }

            if (inNew) {
                auto event = MapEvent::MakeInFrustumEvent(item, &mCurrBaseTiles[item].mPayload);
                pushToSyncEvents(event);
            }
        }

        for (auto &item: diff) {
            bool inNew = mCurrBaseTiles.count(item) > 0;

            if (inNew) {
                std::string url{"http://tile.openstreetmap.org/" + mCurrBaseTiles[item].tileURL() + ".png"};
                auto request = new NetworkRequest{
                        url,
                        [this, item](const std::vector<uint8_t> &data) {
                            auto image = KCore::STBImageUtils::decodeImageBuffer(data);

                            auto raw = new uint8_t[image.size()];
                            std::copy(image.begin(), image.end(), raw);

                            auto event = MapEvent::MakeImageLoadedEvent(
                                    item, raw
                            );
                            pushToAsyncEvents(event);
                        }, nullptr
                };
                mNetworkContext.pushRequestToQueue(request);
            }
        }
    }

    extern "C" {
    DllExport KCore::PlainWorld *CreatePlainWorld(float latitude, float longitude) {
        return new KCore::PlainWorld(latitude, longitude);
    }
    }
}