#pragma once

#include <thread>
#include <memory>
#include <vector>
#include <mutex>
#include <map>

#include "LRUCache17.hpp"

#include "../../generics/IContext.hpp"
#include "../../misc/STBImageUtils.hpp"
#include "../events/LayerEvent.hpp"
#include "ImageResult.hpp"

using namespace std::chrono_literals;

namespace KCore {
    class Layer;

    class IRenderContext : public IContext {
    protected:
        Layer *mWorld;

        std::mutex mContextLock;
        std::vector<KCore::TileDescription> mLastKnownTiles;
        lru17::Cache<std::string, std::vector<uint8_t>> mCachedTextures{150, 10};

    public:
        explicit IRenderContext(Layer *world);

        void storeTextureInContext(const std::vector<uint8_t> &data, const std::string &quadcode);

        void setCurrentTileState(const std::vector<KCore::TileDescription> &tiles);

        void runRenderLoop();

    protected:
        const std::vector<KCore::TileDescription> &getCurrentTileState();
    };
}