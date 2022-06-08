#include "DebugRenderContext.hpp"

#include "misc/DebugResource.inl"
#include "../../Layer.hpp"

namespace KCore::Debug {
    void DebugContext::initialize() {}

    void DebugContext::performLoopStep() {
        if (mWorld->imageEventsCount() != 0) {
            std::this_thread::sleep_for(100ms);
            return;
        }

        auto tiles = getCurrentTileState();

        for (const auto &tile: tiles) {
            std::thread([this, tile]() {
                auto data = Renderer::Debug::BuiltIn::image;

                int width = -1, height = -1, channels = -1;
                auto results = STBImageUtils::decodeImageBuffer(data.data(), data.size(), width, height, channels);

                auto image = new ImageResult{width, height, channels, results};
                auto rootQuadcode = tile.getQuadcode();
                mWorld->pushToCoreEvents(LayerEvent::MakeImageEvent(rootQuadcode, image));
            }).detach();

            std::this_thread::sleep_for(10ms);
        }

        std::this_thread::sleep_for(100ms);
    }

    void DebugContext::dispose() {}
}