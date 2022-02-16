
#include "../BaseWorld.hpp"
#include "Stage.hpp"


namespace KCore {
    Stage::Stage(std::function<void(BaseWorld *)> processor) : mProcessor(std::move(processor)) {}

    void Stage::invoke(BaseWorld *world) {
        mProcessor(world);

    }
}