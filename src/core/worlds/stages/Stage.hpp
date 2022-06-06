//#pragma once
//
//#include <functional>
//#include <utility>
//#include <iostream>
//
//namespace KCore {
//    class IWorld;
//
//    class Stage {
//    private:
//        std::function<void(IWorld *, Stage *)> mStageFunction =
//                [](IWorld *, Stage *) {
//                    // do nothing in placeholder
//                };
//
//        Stage *mNext{nullptr};
//
//    public:
//        Stage(std::function<void(IWorld *, Stage *)> processor);
//
//        void invoke(IWorld *world);
//
//        void next(Stage *stage);
//    };
//}