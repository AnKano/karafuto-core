#pragma once

#include "../INetworkContext.hpp"

#include <future>

namespace KCore {
    class MapCore;

    class BasicNetworkContext : public INetworkContext {
    private:
        void initialize() override;

        void performLoopStep() override;

        void dispose() override;

        void fetch(NetworkRequest* task);
    };
}
