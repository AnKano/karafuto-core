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

using namespace std::chrono_literals;

namespace KCore {
    class TerrainedWorld;
    class GenericTile;

    class IRenderContext : public IContext {
    protected:
        TerrainedWorld *mWorldAdapter;

        std::mutex mTexturesLock, mTileStateLock;
        std::vector<KCore::GenericTile *> mCurrentTileState;
        std::map<std::string, std::vector<uint8_t>> mInRAMNotConvertedTextures;

    public:
        IRenderContext(TerrainedWorld* world);

        void storeTextureInContext(const std::vector<uint8_t> &data, const std::string &quadcode);

        void setCurrentTileState(const std::vector<KCore::GenericTile *> &tiles);

        void runRenderLoop();

    protected:
        const std::vector<KCore::GenericTile *> &getCurrentTileState();
    };
}

#endif //KARAFUTO_CORE_IRENDERCONTEXT_HPP
