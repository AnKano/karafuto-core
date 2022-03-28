#pragma once

#include "../INetworkContext.hpp"

namespace KCore {
    class FallbackNetworkContext : public INetworkContext {
    public:
        FallbackNetworkContext() : INetworkContext() {}

    private:
        void dispose() override;

        void init() override;

        void onEachStep() override;
    };
}

