#pragma once

#include <functional>
#include <utility>
#include <iostream>

namespace KCore {
    class BaseWorld;

    class Stage {
    private:
        std::function<void(BaseWorld *, Stage *)> mStageFunction =
                [](BaseWorld *, Stage *) {
                    // do nothing in placeholder
                };

        Stage *mNext{nullptr};

    public:
        Stage(std::function<void(BaseWorld *, Stage *)> processor);

        void invoke(BaseWorld *world);

        void next(Stage *stage);
    };
}