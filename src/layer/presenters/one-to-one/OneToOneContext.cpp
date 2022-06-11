#include "OneToOneContext.hpp"

#include "../../Layer.hpp"

namespace KCore::OneToOne {
    void OneToOneContext::initialize() {}

    void OneToOneContext::performLoopStep() {
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

            // throttle 10 ms
            std::this_thread::sleep_for(10ms);
        }

        // throttle 10 ms every stage
        std::this_thread::sleep_for(10ms);
    }

    void OneToOneContext::dispose() {}
}