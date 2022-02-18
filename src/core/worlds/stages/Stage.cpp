#include "Stage.hpp"

#include "../BaseWorld.hpp"

namespace KCore {
    Stage::Stage(std::function<void(BaseWorld *, Stage *)> processor) : mStageFunction(std::move(processor)) {}

    void Stage::invoke(BaseWorld *world) {
        mStageFunction(world, mNext);
    }

    void Stage::next(Stage *stage) {
        mNext = stage;
    }
}