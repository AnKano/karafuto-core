#include "NetworkContext.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC

#include <stb_image.h>

#include "../../MapCore.hpp"

namespace KCore {
    NetworkContext::NetworkContext(MapCore *core, BaseCache<std::shared_ptr<void>> *stash, RenderContext *renderContext)
            : mCore_ptr(core), mStash_ptr(stash), mRenderCtx_ptr(renderContext) {
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

    void NetworkContext::pushTaskToQueue(NetworkTask *task) {
        mQueue.pushTask(task);
    }

    void NetworkContext::runRenderLoop() {
        while (!mShouldClose) {
            auto task = mQueue.popTask();
            while (task) {
                putTaskToCURL(task);
                // get next task or nullptr
                task = mQueue.popTask();
            }

            CURLMsg *message;
            auto transmissions = -1;
            while ((message = curl_multi_info_read(mCurlMultiContext, &transmissions))) {
                if (message->msg == CURLMSG_DONE) {
                    CURL *handler = message->easy_handle;

                    TransmissionPayload *payload;
                    curl_easy_getinfo(message->easy_handle, CURLINFO_PRIVATE, &payload);

                    std::string stringTag{payload->quadcode + '.' + payload->tag};

                    int width, height, channels;
                    unsigned char *data = stbi_load_from_memory(
                            (const stbi_uc *) mLoadingResults[stringTag].data(),
                            (int) mLoadingResults[stringTag].size(),
                            &width, &height, &channels, 3
                    );

                    // copy buffer to cache
                    auto buffer = std::make_shared<std::vector<uint8_t>>();
                    buffer->insert(buffer->begin(), data, data + (width * height * channels));

                    // define this buffer in stash
                    mStash_ptr->setOrReplace(stringTag, buffer);

                    MapEvent event{};
                    event.Type = ContentLoadedImage;
                    strcpy_s(event.Quadcode, payload->quadcode.data());
                    event.OptionalPayload = buffer.get();

                    mRenderCtx_ptr->pushTextureDataToGPUQueue(stringTag, buffer);
//                    mCore_ptr->pushEventToContentQueue(event);

                    curl_multi_remove_handle(mCurlMultiContext, handler);
                    curl_easy_cleanup(handler);

                    // erase element from struct
                    mLoadingResults.erase(stringTag);
                    // release memory from payload
                    delete payload;
                }
            }

            curl_multi_perform(mCurlMultiContext, &transmissions);
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

    void NetworkContext::putTaskToCURL(const std::shared_ptr<NetworkTask> &task) {
        auto *payload = new TransmissionPayload{
                &mLoadingResults,
                task->mUrl,
                task->mQuadcode,
                task->mTag
        };

        CURL *curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallbackCURL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, payload);
        curl_easy_setopt(curl, CURLOPT_URL, payload->url.c_str());
        curl_easy_setopt(curl, CURLOPT_PRIVATE, payload);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "KarafutoMapCore/0.0");
        curl_multi_add_handle(mCurlMultiContext, curl);
    }

    void NetworkContext::disposeCURL() {
        curl_multi_cleanup(mCurlMultiContext);
        curl_global_cleanup();
    }

    size_t NetworkContext::writeCallbackCURL(char *data, size_t size, size_t nmemb, TransmissionPayload *payloadPtr) {
        auto &stash_ptr = *payloadPtr->resultsStash_ptr;
        auto &tag = payloadPtr->tag;
        auto &quadcode = payloadPtr->quadcode;

        auto composite = quadcode + '.' + tag;

        if (stash_ptr.find(composite) == std::end(stash_ptr))
            stash_ptr[composite] = {};

        auto &buffer = stash_ptr[composite];
        buffer.insert(buffer.end(), data, data + (size * nmemb));

        return size * nmemb;
    }
}