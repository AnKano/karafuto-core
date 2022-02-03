#include "NetworkContext.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC

#include <stb_image.h>

#include "../../MapCore.hpp"

namespace KCore {
    NetworkContext::NetworkContext(MapCore *core, RenderContext *renderContext)
            : mCore_ptr(core), mRenderCtx_ptr(renderContext) {
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

            auto currentTransmissionsCount = 0;

            CURLMsg *message;
            while ((message = curl_multi_info_read(mCurlMultiContext, &currentTransmissionsCount))) {
                if (message->msg == CURLMSG_DONE) {
                    CURL *handler = message->easy_handle;

                    TransmissionPayload *payload;
                    curl_easy_getinfo(message->easy_handle, CURLINFO_PRIVATE, &payload);

                    std::string stringTag{payload->quadcode + '.' + payload->tag};

                    try {
                        auto decodedBuffer = decodeImage(mLoadingResults[stringTag]);
                        mCore_ptr->mDataStash.setOrReplace(stringTag, decodedBuffer);
                        mRenderCtx_ptr->pushTextureDataToGPUQueue(stringTag, decodedBuffer);
                    } catch (std::exception& e) {
                        std::cerr << e.what() << std::endl;
                    }

                    curl_multi_remove_handle(mCurlMultiContext, handler);
                    curl_easy_cleanup(handler);

                    // erase element from struct
                    mLoadingResults.erase(stringTag);
                    // release memory from payload
                    delete payload;
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

    std::shared_ptr<TransmissionBuffer> NetworkContext::decodeImage(const TransmissionBuffer &buffer) {
        int width, height, channels;
        unsigned char *data = stbi_load_from_memory(
                (const stbi_uc *) buffer.data(), (int) buffer.size(),
                &width, &height, &channels, 3
        );

        if (width < 0 || height < 0 || channels < 0)
            throw std::runtime_error("can't decode image");

        auto out = std::make_shared<std::vector<uint8_t>>();
        out->resize(width * height * channels);
        out->insert(out->begin(), data, data + (width * height * channels));

        return out;
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
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "KarafutoMapCore/0.1");
        curl_multi_add_handle(mCurlMultiContext, curl);
    }

    void NetworkContext::disposeCURL() {
        curl_multi_cleanup(mCurlMultiContext);
        curl_global_cleanup();
    }

    size_t NetworkContext::writeCallbackCURL(char *data, size_t size, size_t nmemb, TransmissionPayload *payloadPtr) {
        auto &transmissionBuffersMap = *payloadPtr->transmissionBuffers;
        auto &tag = payloadPtr->tag;
        auto &quadcode = payloadPtr->quadcode;

        auto transmittedBytes = size * nmemb;

        // calculate composite tag i.e. '133122222.common.image' where '.' is separator
        auto composite{quadcode + '.' + tag};

        // create buffer for transmissions if it's not founded in stash
        if (!transmissionBuffersMap.count(composite)) transmissionBuffersMap[composite] = {};

        auto &buffer = transmissionBuffersMap[composite];
        buffer.insert(buffer.end(), data, data + (transmittedBytes));

        return transmittedBytes;
    }
}