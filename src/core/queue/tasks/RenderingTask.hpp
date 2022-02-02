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
    public:
        MapCore *mCore_ptr;
        std::string mQuadcode;
        std::vector<std::string> mChilds;

    public:
        RenderingTask(MapCore *mCore, std::string quadcode, std::vector<std::string> childs) :
                mCore_ptr(mCore), mQuadcode(std::move(quadcode)), mChilds(std::move(childs)) {}

        void performTask() override;

        void onTaskComplete() override;
    };
}