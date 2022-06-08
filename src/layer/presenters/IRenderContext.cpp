#include "IRenderContext.hpp"

#include "../Layer.hpp"

namespace KCore {
    IRenderContext::IRenderContext(Layer* world) : mWorld(world) {
        mThread = std::make_unique<std::thread>([this]() {
            initialize();
            runRenderLoop();
        });
        mThread->detach();
    }

    void IRenderContext::storeTextureInContext(const std::vector<uint8_t> &data, const std::string &quadcode) {
        std::lock_guard<std::mutex> lock{mContextLock};
        mCachedTextures[quadcode] = data;
    }

    void IRenderContext::runRenderLoop() {
        while (!mShouldClose) {
            if (mWorld->imageEventsCount() != 0) {
                std::this_thread::sleep_for(100ms);
                continue;
            }

            performLoopStep();

            std::this_thread::sleep_for(100ms);
        }

        dispose();
        mReadyToBeDead = true;
    }

    void IRenderContext::setCurrentTileState(const std::vector<KCore::TileDescription> &tiles) {
        std::lock_guard<std::mutex> lock{mContextLock};
        mLastKnownTiles = tiles;
    }

    const std::vector<KCore::TileDescription> &IRenderContext::getCurrentTileState() {
        std::lock_guard<std::mutex> lock{mContextLock};
        return mLastKnownTiles;
    }

    void IRenderContext::clearCached() {
        std::lock_guard<std::mutex> lock{mContextLock};
        mCachedTextures.clear();
    }
}