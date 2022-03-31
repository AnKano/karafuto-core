//
// Created by Anton Shubin on 3/15/22.
//

#include "IRenderContext.hpp"
#include "../../worlds/TerrainedWorld.hpp"

#include <gzip/compress.hpp>

namespace KCore {
    IRenderContext::IRenderContext(TerrainedWorld *world) : mWorldAdapter(world) {
        mRenderThread = std::make_unique<std::thread>([this]() {
            initialize();
            runRenderLoop();
        });
        mRenderThread->detach();
    }

    IRenderContext::~IRenderContext() {
        setShouldClose(true);
        // await to thread stop working
        while (getWorkingStatus()) std::this_thread::sleep_for(10ms);
    }

    void IRenderContext::setCheckInterval(const uint64_t &value) {
        mCheckInterval = std::chrono::milliseconds(value);
    }

    void IRenderContext::setCheckInterval(const std::chrono::milliseconds &value) {
        mCheckInterval = value;
    }

    void IRenderContext::setShouldClose(const bool &value) {
        mShouldClose = value;
    }

    bool IRenderContext::getWorkingStatus() const {
        return mReadyToBeDead;
    }

    void IRenderContext::storeTextureInContext(const std::vector<uint8_t> &data, const std::string &quadcode) {
        std::lock_guard<std::mutex> lock{mTexturesLock};

        std::string compressed_data = gzip::compress(reinterpret_cast<const char *>(data.data()), data.size(), Z_BEST_SPEED);
        std::vector<uint8_t> conv;
        std::copy(compressed_data.begin(), compressed_data.end(), std::back_inserter(conv));

//        std::cout << "compressed: " << conv.size() << std::endl;

        mInRAMNotConvertedTextures[quadcode] = data;
    }

    void IRenderContext::runRenderLoop() {
        while (!mShouldClose) {
            if (mWorldAdapter->getAsyncEventsLength() != 0) {
                std::this_thread::sleep_for(350ms);
                continue;
            }

            performLoopStep();
        }

        dispose();
        mReadyToBeDead = true;
    }

    void IRenderContext::setCurrentTileState(const std::vector<KCore::GenericTile *> &tiles) {
        std::lock_guard<std::mutex> lock{mTileStateLock};
        mCurrentTileState = tiles;
    }

    const std::vector<KCore::GenericTile *> &IRenderContext::getCurrentTileState() {
        std::lock_guard<std::mutex> lock{mTileStateLock};
        return mCurrentTileState;
    }
}