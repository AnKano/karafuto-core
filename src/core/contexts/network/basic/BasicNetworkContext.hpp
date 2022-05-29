#pragma once

#include "../INetworkContext.hpp"

#include <future>

namespace KCore {
    class MapCore;

    class BasicNetworkContext : public INetworkContext {
    public:
        void synchronousStep() override;

    private:
        std::vector<std::future<void>> pending_futures;

        void initialize() override;

        void performLoopStep() override;

        void dispose() override;
    };
}
