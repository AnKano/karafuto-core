#pragma once

#include <stdexcept>
#include <thread>
#include <chrono>
#include <iostream>
#include <map>

#include "../../queue/Queue.hpp"
#include "../../geography/TileDescription.hpp"
#include "../../queue/tasks/NetworkTask.hpp"
#include "../../cache/BaseCache.hpp"

#include "NetworkRequest.hpp"
#include "../IContext.hpp"

using namespace std::chrono_literals;

namespace KCore {
    class MapCore;

    class INetworkContext : public IContext {
    protected:
        std::deque<NetworkRequest *> mRequestQueue;

    public:
        INetworkContext();

        void pushRequestToQueue(NetworkRequest* request);

    private:
        void runLoop();

        virtual void initialize() = 0;

        virtual void performLoopStep() = 0;

        virtual void dispose() = 0;

        void disposeContext();
    };
}