#pragma once

#include <curl/curl.h>

#include "../NetworkRequest.hpp"
#include "../INetworkContext.hpp"

using namespace std::chrono_literals;

namespace KCore {
    class MapCore;

    class CURLNetworkContext : public INetworkContext {
    private:
        CURLM *mCurlMultiContext{};

    private:
        void putTaskToCURL(NetworkRequest *task);

        static size_t writeCallbackCURL(char *data, size_t size, size_t nmemb, void *payloadPtr);

    // overrides described below

    private:
        void initialize() override;

        void performLoopStep() override;

        void dispose() override;

    public:
        void synchronousStep() override;
    };
}