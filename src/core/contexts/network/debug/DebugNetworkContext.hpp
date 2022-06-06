#pragma once

#include "../INetworkContext.hpp"

namespace KCore {
    /**
     * Simulate downloading of 64 x 64 RGB888 debug image for testing purposes
     */
    class [[maybe_unused]] DebugNetworkContext : public INetworkContext {
    public:
        DebugNetworkContext() : INetworkContext() {}

    private:
        void dispose() override;

        void initialize() override;

        void performLoopStep() override;
    };
}

