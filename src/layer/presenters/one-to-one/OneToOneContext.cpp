#include "OneToOneContext.hpp"

#include "../../Layer.hpp"

namespace KCore::OneToOne {
    void OneToOneContext::initialize() {}

    void OneToOneContext::performLoopStep() {
        if (mWorld->imageEventsCount() != 0) {
            std::this_thread::sleep_for(100ms);
            return;
        }

        auto tiles = getCurrentTileState();

        for (const auto &tile: tiles) {
            auto relatedCode = tile.getRelatedQuadcodes()[0];
            if (!mCachedTextures.contains(relatedCode)) continue;

            std::thread([this, tile, relatedCode]() {
                auto data = mCachedTextures[relatedCode];

                int width = -1, height = -1, channels = -1;
                auto results = STBImageUtils::decodeImageBuffer(data.data(), data.size(), width, height, channels);
                auto image = new ImageResult{width, height, channels, results};
                auto rootQuadcode = tile.getQuadcode();
                mWorld->pushToImageEvents(LayerEvent::MakeImageEvent(rootQuadcode, image));
            }).detach();
        }

        std::this_thread::sleep_for(100ms);
    }

    void OneToOneContext::dispose() {}
}