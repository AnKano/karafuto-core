//
// Created by Anton Shubin on 3/15/22.
//

#include "IRenderContext.hpp"

#include "../../World.hpp"

namespace KCore {
    IRenderContext::IRenderContext(World* world) : mWorld(world) {
        mThread = std::make_unique<std::thread>([this]() {
            initialize();
            runRenderLoop();
        });
        mThread->detach();
    }

    void IRenderContext::storeTextureInContext(const std::vector<uint8_t> &data, const std::string &quadcode) {
        std::lock_guard<std::mutex> lock{mTexturesLock};
        mInRAMNotConvertedTextures[quadcode] = data;
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
        std::lock_guard<std::mutex> lock{mTileStateLock};
        mCurrentTileState = tiles;
    }

    const std::vector<KCore::TileDescription> &IRenderContext::getCurrentTileState() {
        std::lock_guard<std::mutex> lock{mTileStateLock};
        return mCurrentTileState;
    }
}