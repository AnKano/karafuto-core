#pragma once

#include <stdexcept>
#include <thread>
#include <chrono>
#include <iostream>
#include <map>

#include <curl/curl.h>

#include "../../queue/Queue.hpp"
#include "../../geography/TileDescription.hpp"
#include "../../queue/tasks/NetworkTask.hpp"
#include "../../cache/BaseCache.hpp"
#include "../rendering/RenderContext.hpp"
#include "NetworkRequest.hpp"

using namespace std::chrono_literals;

namespace KCore {
    class MapCore;

    class NetworkContext {
    private:
        CURLM *mCurlMultiContext{};

        Queue<NetworkTask> mQueue;
        std::deque<NetworkRequest *> mRequestQueue;

        std::unique_ptr<std::thread> mRenderThread;
        std::chrono::milliseconds mWaitInterval = 1s;
        bool mShouldClose = false;
        bool mReadyToBeDead = false;

    public:
        NetworkContext();

        ~NetworkContext();

        [[maybe_unused]]
        void setWaitInterval(const uint64_t &value);

        [[maybe_unused]]
        void setWaitInterval(const std::chrono::milliseconds &value);

        void setShouldClose(const bool &value);

        [[nodiscard]]
        bool getWorkingStatus() const;

        void pushRequestToQueue(NetworkRequest *task);

    private:
        void runRenderLoop();

        void disposeContext();

        void initCURL();

        void putTaskToCURL(NetworkRequest* task);

        void disposeCURL();

        static size_t writeCallbackCURL(char *data, size_t size, size_t nmemb, void *payloadPtr);
    };
}