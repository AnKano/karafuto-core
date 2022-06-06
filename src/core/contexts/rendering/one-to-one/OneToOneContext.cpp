#include "OneToOneContext.hpp"

#include "../../../World.hpp"

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
            if (!mInRAMNotConvertedTextures.contains(relatedCode)) continue;

            std::thread([this, tile, relatedCode]() {
                auto data = mInRAMNotConvertedTextures[relatedCode];

                int width = -1, height = -1, channels = -1;
                auto results = STBImageUtils::decodeImageBuffer(data.data(), data.size(), width, height, channels);

                auto image = new ImageResult{};
                image->width = width;
                image->height = height;
                image->size = width * height * channels;
                if (channels == 3)
                    image->format = RGB888;
                else if (channels == 4)
                    image->format = RGBA8888;

                image->data = new uint8_t[image->size];
                std::copy(results.begin(), results.end(), image->data);

                auto rootQuadcode = tile.getQuadcode();
                mWorld->pushToImageEvents(Event::MakeImageEvent(rootQuadcode, image));
            }).detach();
        }

        std::this_thread::sleep_for(100ms);
    }

    void OneToOneContext::dispose() {}
}