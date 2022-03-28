#pragma once

#include <curl/curl.h>

#include "NetworkRequest.hpp"
#include "../INetworkContext.hpp"

using namespace std::chrono_literals;

namespace KCore {
    class MapCore;

    class CURLNetworkContext : public INetworkContext {
    private:
        CURLM *mCurlMultiContext{};

        Queue<NetworkTask> mQueue;
        std::deque<NetworkRequest *> mRequestQueue;

        std::unique_ptr<std::thread> mRenderThread;
        std::chrono::milliseconds mWaitInterval = 1s;
        bool mShouldClose = false;
        bool mReadyToBeDead = false;

    public:
        CURLNetworkContext() : INetworkContext() {}

        void pushRequestToQueue(NetworkRequest *task);

    private:
        void dispose() override;

        void init() override;

        void onEachStep() override;

        void putTaskToCURL(NetworkRequest *task);

        void disposeCURL();

        static size_t writeCallbackCURL(char *data, size_t size, size_t nmemb, void *payloadPtr);
    };
}