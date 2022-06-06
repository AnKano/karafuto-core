#include "FallbackRenderContext.hpp"

#include "misc/FallbackResource.inl"
#include "../../../World.hpp"

namespace KCore::Fallback {
    void FallbackRenderContext::initialize() {}

    void FallbackRenderContext::performLoopStep() {
        if (mWorld->imageEventsCount() != 0) {
            std::this_thread::sleep_for(100ms);
            return;
        }

        auto tiles = getCurrentTileState();

        for (const auto &tile: tiles) {
            auto data = Rendering::Fallback::BuiltIn::image;

            int width = -1, height = -1, channels = -1;
            auto results = STBImageUtils::decodeImageBuffer(data.data(), data.size(), width, height, channels);

            std::thread([results, this, tile, width, height]() {
                auto t0 = std::chrono::high_resolution_clock::now();

                auto image = new ImageResult{};
                image->width = width;
                image->height = height;
                image->size = width * height * 3;
                image->format = RGB888;
                image->data = new uint8_t[image->size];
                std::copy(results.begin(), results.end(), image->data);

                auto rootQuadcode = tile.getQuadcode();
                mWorld->pushToCoreEvents(Event::MakeImageEvent(rootQuadcode, image));
                auto t1 = std::chrono::high_resolution_clock::now();
                auto duration = t1 - t0;

                auto d = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
            }).detach();

            std::this_thread::sleep_for(10ms);
        }

        std::this_thread::sleep_for(100ms);
    }

    void FallbackRenderContext::dispose() {}
}