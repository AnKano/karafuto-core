//
// Created by Anton Shubin on 3/15/22.
//

#ifndef KARAFUTO_CORE_IRENDERCONTEXT_HPP
#define KARAFUTO_CORE_IRENDERCONTEXT_HPP

#include <thread>
#include <memory>
#include <vector>
#include <mutex>
#include <map>

using namespace std::chrono_literals;

namespace KCore {
    class TerrainedWorld;
    class GenericTile;

    class IRenderContext {
    protected:
        TerrainedWorld *mWorldAdapter;

        std::mutex mTexturesLock, mTileStateLock;
        std::vector<KCore::GenericTile *> mCurrentTileState;
        std::map<std::string, std::vector<uint8_t>> mInRAMNotConvertedTextures;

        // thread specific variables
        std::unique_ptr<std::thread> mRenderThread;

        std::chrono::milliseconds mCheckInterval = 1s;
        bool mShouldClose = false;
        bool mReadyToBeDead = false;

    public:
        IRenderContext(TerrainedWorld* world);

        ~IRenderContext();

        [[maybe_unused]]
        void setCheckInterval(const uint64_t &value);

        [[maybe_unused]]
        void setCheckInterval(const std::chrono::milliseconds &value);

        void setShouldClose(const bool &value);

        [[nodiscard]]
        bool getWorkingStatus() const;

        void storeTextureInContext(const std::vector<uint8_t> &data, const std::string &quadcode);

        void setCurrentTileState(const std::vector<KCore::GenericTile *> &tiles);

        void runRenderLoop();

    protected:
        const std::vector<KCore::GenericTile *> &getCurrentTileState();

        virtual void initialize() = 0;

        virtual void performLoopStep() = 0;

        virtual void dispose() = 0;
    };
}

#endif //KARAFUTO_CORE_IRENDERCONTEXT_HPP
