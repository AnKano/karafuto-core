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

using namespace std::chrono_literals;

namespace KCore {
    class MapCore;

    class INetworkContext {
    protected:
        std::deque<NetworkRequest *> mRequestQueue;

        std::unique_ptr<std::thread> mRenderThread;
        std::chrono::milliseconds mWaitInterval = 1s;
        bool mShouldClose = false;
        bool mReadyToBeDead = false;

    public:
        INetworkContext();

        ~INetworkContext();

        [[maybe_unused]]
        void setWaitInterval(const uint64_t &value);

        [[maybe_unused]]
        void setWaitInterval(const std::chrono::milliseconds &value);

        void setShouldClose(const bool &value);

        [[nodiscard]]
        bool getWorkingStatus() const;

        void pushRequestToQueue(NetworkRequest* request);

        virtual void synchronousStep();

    private:
        void runLoop();

        virtual void initialize() = 0;

        virtual void performLoopStep() = 0;

        virtual void dispose() = 0;

        void disposeContext();
    };
}