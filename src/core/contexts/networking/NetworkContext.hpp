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

using namespace std::chrono_literals;

namespace KCore {
    class MapCore;

    typedef std::string TransmissionKey;
    typedef std::vector<uint8_t> TransmissionBuffer;

    class NetworkContext {
        /*
         * CURL library need to send anything that described destination
         * for loaded bytes. The best way is creating struct that contain
         * map to each one result (for easily appending) and description
         * for certain handler to describe key
         * */
        struct TransmissionPayload {
            std::map<TransmissionKey, TransmissionBuffer> *resultsStash_ptr;
            std::string url;
            std::string quadcode;
            std::string tag;
        };

    private:
        MapCore *mCore_ptr;
        RenderContext* mRenderCtx_ptr;

        CURLM *mCurlMultiContext;

        std::map<TransmissionKey, TransmissionBuffer> mLoadingResults;

        BaseCache<std::shared_ptr<void>> *mStash_ptr;
        Queue<NetworkTask> mQueue;

        std::unique_ptr<std::thread> mRenderThread;
        std::chrono::milliseconds mWaitInterval = 1s;
        bool mShouldClose = false;
        bool mReadyToBeDead = false;

    public:
        NetworkContext(MapCore *core, BaseCache<std::shared_ptr<void>> *stash, RenderContext* renderContext);

        ~NetworkContext();

        [[maybe_unused]]
        void setWaitInterval(const uint64_t &value);

        [[maybe_unused]]
        void setWaitInterval(const std::chrono::milliseconds &value);

        void setShouldClose(const bool &value);

        [[nodiscard]]
        bool getWorkingStatus() const;

        void pushTaskToQueue(NetworkTask *task);

    private:
        void runRenderLoop();

        void disposeContext();

        void initCURL();

        void putTaskToCURL(const std::shared_ptr<NetworkTask> &task);

        void disposeCURL();

        static size_t writeCallbackCURL(char *data, size_t size, size_t nmemb, TransmissionPayload *payloadPtr);
    };
}