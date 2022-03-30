#pragma once

#include "../INetworkContext.hpp"

namespace KCore {
    class EmscriptenFetchNetworkContext : public INetworkContext {
    public:
        EmscriptenFetchNetworkContext() : INetworkContext() {}

        void synchronousStep() override;

    private:
        static void performTask(NetworkRequest *task);

        void dispose() override;

        void initialize() override;

        void performLoopStep() override;
    };
}