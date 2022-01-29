#pragma once

#include <map>
#include <string>
#include <memory>

#include "../../cache/BaseCache.hpp"
#include "../../cache/LimitedCache.hpp"
#include "../TileDescription.hpp"
#include "../../rendering/RenderContext.hpp"
#include "../../meshes/BaseMesh.hpp"
#include "../../queue/tasks/NetworkTask.hpp"
#include "../../meshes/GridMesh.hpp"

namespace KCore {
    class CommonTile {
    private:
        LimitedCache<std::map<std::string, std::shared_ptr<void>>> *mTarget{};
        RenderContext *mQueueContext{};

        TileDescription mDescription;

        std::shared_ptr<BaseMesh> mMesh{};
        std::shared_ptr<std::vector<uint8_t>> mImage{};

        bool mReady = false;

    public:
        CommonTile() = default;

        CommonTile(LimitedCache<std::map<std::string, std::shared_ptr<void>>> *cache,
                   RenderContext *context,
                   const TileDescription &description) :
                mTarget(cache), mDescription(description), mQueueContext(context) {
            produceTasks();
        };

        void produceTasks() {
            // create
            mQueueContext->pushTaskToQueue(new NetworkTask(
                    mTarget,
                    mDescription.getQuadcode(),
                    mDescription.tileURL())
            );

            mMesh = std::make_shared<GridMesh>(1.0, 1.0, 64);
        }

        void updateRelatedFields() {
            if (mReady) return;

            auto inCache = mTarget->getByKey(mDescription.getQuadcode());
            if (!inCache) return;

            auto terrain = std::static_pointer_cast<std::vector<uint8_t>>((*inCache)["terrain"]);
            if (!terrain) return;

            mImage = terrain;
//            std::cout << "tile " << mDescription.getQuadcode() << " ready" << std::endl;
            mReady = true;
        }

        [[nodiscard]]
        const std::shared_ptr<BaseMesh> &getMesh() const {
            return mMesh;
        }

        [[nodiscard]]
        const std::shared_ptr<std::vector<uint8_t>> &getImage() const {
            return mImage;
        }

        [[nodiscard]]
        const TileDescription &getDescription() const {
            return mDescription;
        }

        [[nodiscard]]
        bool isReady() const {
            return mReady;
        }
    };
}