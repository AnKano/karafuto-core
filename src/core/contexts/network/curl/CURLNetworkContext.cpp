#include "CURLNetworkContext.hpp"

#include "../../MapCore.hpp"

namespace KCore {
    void CURLNetworkContext::pushRequestToQueue(NetworkRequest *task) {
        mRequestQueue.push_back(task);
    }

    void CURLNetworkContext::onEachStep() {
        while (!mRequestQueue.empty()) {
            // get next task or nullptr
            auto task = mRequestQueue.back();
            mRequestQueue.pop_back();
            putTaskToCURL(task);
        }

        auto currentTransmissionsCount = 0;

        CURLMsg *message;
        while ((message = curl_multi_info_read(mCurlMultiContext, &currentTransmissionsCount))) {
            if (message->msg == CURLMSG_DONE) {
                NetworkRequest *request;
                curl_easy_getinfo(message->easy_handle, CURLINFO_PRIVATE, &request);
                CURL *handler = message->easy_handle;

                request->Finalize();

                curl_multi_remove_handle(mCurlMultiContext, handler);
                curl_easy_cleanup(handler);
            }
        }

        curl_multi_perform(mCurlMultiContext, &currentTransmissionsCount);
        curl_multi_wait(mCurlMultiContext, nullptr, 0,
                        (int) mWaitInterval.count(), nullptr);
    }

    void CURLNetworkContext::init() {
        curl_global_init(CURL_GLOBAL_ALL);
        mCurlMultiContext = curl_multi_init();
    }

    void CURLNetworkContext::putTaskToCURL(NetworkRequest *task) {
        CURL *curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallbackCURL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &task->getBuffer());
        curl_easy_setopt(curl, CURLOPT_URL, task->getUrl().c_str());
        curl_easy_setopt(curl, CURLOPT_PRIVATE, task);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "KarafutoMapCore/0.1");
        curl_multi_add_handle(mCurlMultiContext, curl);
    }

    void CURLNetworkContext::dispose() {
        curl_multi_cleanup(mCurlMultiContext);
        curl_global_cleanup();
    }

    size_t CURLNetworkContext::writeCallbackCURL(char *data, size_t size, size_t nmemb, void *buffer) {
        auto transmissionBuffer = (std::vector<uint8_t> *) buffer;
        auto transmittedBytes = size * nmemb;

        transmissionBuffer->insert(transmissionBuffer->end(), data, data + (transmittedBytes));

        return transmittedBytes;
    }
}