#include "NetworkContext.hpp"

#include "../../MapCore.hpp"

namespace KCore {
    NetworkContext::NetworkContext() {
        mRenderThread = std::make_unique<std::thread>([this]() {
            initCURL();
            runRenderLoop();
        });
        mRenderThread->detach();
    }

    NetworkContext::~NetworkContext() {
        disposeContext();
    }

    void NetworkContext::setWaitInterval(const uint64_t &value) {
        mWaitInterval = std::chrono::milliseconds(value);
    }

    void NetworkContext::setWaitInterval(const std::chrono::milliseconds &value) {
        mWaitInterval = value;
    }

    void NetworkContext::setShouldClose(const bool &value) {
        mShouldClose = value;
    }

    bool NetworkContext::getWorkingStatus() const {
        return mReadyToBeDead;
    }

    void NetworkContext::pushRequestToQueue(NetworkRequest *task) {
        mRequestQueue.push_back(task);
    }

    void NetworkContext::runRenderLoop() {
        while (!mShouldClose) {
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

        disposeContext();

        mReadyToBeDead = true;
    }

    void NetworkContext::disposeContext() {
        setShouldClose(true);
        // await to thread stop working
        while (getWorkingStatus()) std::this_thread::sleep_for(10ms);

        disposeCURL();
    }

    void NetworkContext::initCURL() {
        curl_global_init(CURL_GLOBAL_ALL);
        mCurlMultiContext = curl_multi_init();
    }

    void NetworkContext::putTaskToCURL(NetworkRequest *task) {
        CURL *curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallbackCURL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &task->getBuffer());
        curl_easy_setopt(curl, CURLOPT_URL, task->getUrl().c_str());
        curl_easy_setopt(curl, CURLOPT_PRIVATE, task);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "KarafutoMapCore/0.1");
        curl_multi_add_handle(mCurlMultiContext, curl);
    }

    void NetworkContext::disposeCURL() {
        curl_multi_cleanup(mCurlMultiContext);
        curl_global_cleanup();
    }

    size_t NetworkContext::writeCallbackCURL(char *data, size_t size, size_t nmemb, void *buffer) {
        auto transmissionBuffer = (std::vector<uint8_t> *) buffer;
        auto transmittedBytes = size * nmemb;

        transmissionBuffer->insert(transmissionBuffer->end(), data, data + (transmittedBytes));

        return transmittedBytes;
    }
}