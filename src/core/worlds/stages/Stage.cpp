#include "Stage.hpp"

#include "../BaseWorld.hpp"

namespace KCore {
    Stage::Stage(std::function<void(BaseWorld *)> processor) : mProcessor(std::move(processor)) {}

    void Stage::invoke(BaseWorld *world) {
        mProcessor(world);
    }
}