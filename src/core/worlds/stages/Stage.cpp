//#include "Stage.hpp"
//
//#include "../IWorld.hpp"
//
//namespace KCore {
//    Stage::Stage(std::function<void(IWorld *, Stage *)> processor) : mStageFunction(std::move(processor)) {}
//
//    void Stage::invoke(IWorld *world) {
//        mStageFunction(world, mNext);
//    }
//
//    void Stage::next(Stage *stage) {
//        mNext = stage;
//    }
//}