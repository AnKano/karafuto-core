#pragma once

#include <string>
#include <utility>
#include <iostream>

#include <curl/curl.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC

#include <stb_image.h>

#include "BaseTask.hpp"

namespace KCore {
    class NetworkTask : public BaseTask {
    private:
        BaseCache <std::map<std::string, std::shared_ptr<void>>> *mTarget;
        std::map<std::string, std::shared_ptr<void>> mCacheCopy;

        std::string mQuadcode;
        std::string mPayload;

        std::string mCurlBuffer;
        CURLcode mCurlResult;

    public:
        NetworkTask(BaseCache<std::map<std::string, std::shared_ptr<void>>> *dataCache,
                    std::string quadcode,
                    std::string payload) :
                mTarget(dataCache),
                mQuadcode(std::move(quadcode)),
                mPayload(std::move(payload)) {
            auto cache = mTarget->getByKey(mQuadcode);
            if (!cache) {
                // check dataCache existence
                mTarget->setOrReplace(mQuadcode, {});
                cache = mTarget->getByKey(mQuadcode);
            }

            mCacheCopy = std::map<std::string, std::shared_ptr<void>>(*cache);
        }

        void performTask() override {
            std::string url{"http://tile.openstreetmap.org/" + mPayload + ".png"};
            const char *cUrl = url.c_str();
            char curlErrorBuffer[CURL_ERROR_SIZE];

            CURL *curl = curl_easy_init();
            if (!curl) return;

            curl_easy_setopt(curl, CURLOPT_USERAGENT, "KarafutoMapCore/0.0");
            curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
            curl_easy_setopt(curl, CURLOPT_URL, cUrl);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mCurlBuffer);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFunc);

            mCurlResult = curl_easy_perform(curl);

            curl_easy_cleanup(curl);
        }

        void onTaskComplete() override {
            int x, y, n;
            unsigned char *data = stbi_load_from_memory((const stbi_uc *) mCurlBuffer.data(),
                                                        (int) mCurlBuffer.size(),
                                                        &x, &y, &n, 3);

            auto rawSize = x * y * n; // width*height*channels
            // copy buffer to cache
            auto buffer = std::make_shared<std::vector<uint8_t>>();
            buffer->resize(rawSize);
            memcpy(buffer->data(), data, rawSize);

            // restore cache state
            mCacheCopy["terrain"] = std::static_pointer_cast<void>(buffer);

            mTarget->setOrReplace(mQuadcode, mCacheCopy);

//            std::cout << "Load complete! " << buffer->size() << " bytes" << std::endl;
        }

    private:
        static size_t curlWriteFunc(char *data, size_t size, size_t nmemb, std::string *buffer) {
            size_t result = 0;

            if (buffer != nullptr) {
                buffer->append(data, size * nmemb);
                result = size * nmemb;
            }

            return result;
        }
    };
}