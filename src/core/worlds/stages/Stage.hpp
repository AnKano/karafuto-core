#pragma once

#include <functional>
#include <utility>
#include <iostream>

namespace KCore {
    class BaseWorld;

    class Stage {
    private:
        std::function<void(BaseWorld *)> mProcessor =
                [](BaseWorld *) {
                    // do nothing in placeholder
                };

    public:
        Stage(std::function<void(BaseWorld *)> processor);

        void invoke(BaseWorld *world);
    };
}