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

#include "../IContext.hpp"
#include "../../misc/STBImageUtils.hpp"
#include "../../events/Event.hpp"
#include "ImageResult.hpp"

using namespace std::chrono_literals;

namespace KCore {
    class World;

    class IRenderContext : public IContext {
    protected:
        World *mWorld;

        std::mutex mTexturesLock, mTileStateLock;
        std::vector<KCore::TileDescription> mCurrentTileState;
        std::map<std::string, std::vector<uint8_t>> mInRAMNotConvertedTextures;

    public:
        IRenderContext(World *world);

        void storeTextureInContext(const std::vector<uint8_t> &data, const std::string &quadcode);

        void setCurrentTileState(const std::vector<KCore::TileDescription> &tiles);

        void runRenderLoop();

    protected:
        const std::vector<KCore::TileDescription> &getCurrentTileState();
    };
}

#endif //KARAFUTO_CORE_IRENDERCONTEXT_HPP
