#pragma once

#include <string>
#include <utility>
#include <iostream>

#include "../../cache/BaseCache.hpp"
#include "../../events/MapEvent.hpp"

#include "BaseTask.hpp"

namespace KCore {
    class MapCore;

    class RenderingTask : public BaseTask {
    private:
        MapCore *mCore_ptr;
        BaseCache<std::shared_ptr<void>> *mStash_ptr;
        std::string mQuadcode;

    public:
        RenderingTask(MapCore *mCore, BaseCache<std::shared_ptr<void>> *stash,
                      std::string relQuadcode) : mCore_ptr(mCore), mStash_ptr(stash),
                                                 mQuadcode(std::move(relQuadcode)) {}

        void performTask() override;

        void onTaskComplete() override;
    };
}