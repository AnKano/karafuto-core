#pragma once

#include "../INetworkContext.hpp"

namespace KCore {
    class [[maybe_unused]] FallbackNetworkContext : public INetworkContext {
    public:
        FallbackNetworkContext() : INetworkContext() {}

    private:
        void dispose() override;

        void initialize() override;

        void performLoopStep() override;
    };
}

