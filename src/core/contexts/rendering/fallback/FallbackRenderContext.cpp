#include "FallbackRenderContext.hpp"

#include "../../../worlds/TerrainedWorld.hpp"
#include "misc/FallbackResource.inl"

namespace KCore::Fallback {
    void FallbackRenderContext::initialize() {

    }

    void FallbackRenderContext::performLoopStep() {
        if (mWorldAdapter->getAsyncEventsLength() != 0) {
            std::this_thread::sleep_for(350ms);
            return;
        };

        auto metas = getCurrentTileState();

        for (const auto &meta: metas) {
            auto data = Rendering::Fallback::BuiltIn::image;

            int width = -1, height = -1, channels = -1;
            auto results = STBImageUtils::decodeImageBuffer(data.data(), data.size(), width, height, channels);

            std::thread([results, this, meta]() {
                auto t0 = std::chrono::high_resolution_clock::now();

                auto rawBuffer = new std::vector<uint8_t>{};
                rawBuffer->resize(results.size());
                std::copy(results.begin(), results.end(), rawBuffer->data());

                auto rootQuadcode = meta->getTileDescription().getQuadcode();
                mWorldAdapter->pushToAsyncEvents(MapEvent::MakeRenderLoadedEvent(rootQuadcode, rawBuffer));
                auto t1 = std::chrono::high_resolution_clock::now();
                auto duration = t1 - t0;

                auto d = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
            }).detach();

            std::this_thread::sleep_for(10ms);
        }
    }

    void FallbackRenderContext::dispose() {

    }
}