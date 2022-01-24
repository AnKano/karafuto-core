#pragma once

#include <string>
#include <utility>
#include <iostream>

#include "curl/curl.h"

#include "BaseTask.hpp"

namespace KCore {
    class NetworkTask : public BaseTask {
    private:
        TimeoutCache <std::map<std::string, uint8_t *>> *mTarget;

        std::string mQuadcode;
        std::string mPayload;

        std::string mCurlBuffer;
        CURLcode mCurlResult;

    public:
        NetworkTask(TimeoutCache <std::map<std::string, uint8_t *>> *cache,
                    std::string quadcode,
                    std::string payload) :
                mTarget(cache),
                mQuadcode(std::move(quadcode)),
                mPayload(std::move(payload)) {}

        void performTask() override {
            std::string url{"http://tile.openstreetmap.org/" + mPayload + ".png"};
            const char *c_url = url.c_str();
            char curlErrorBuffer[CURL_ERROR_SIZE];

            CURL *curl = curl_easy_init();
            if (!curl) return;

            curl_easy_setopt(curl, CURLOPT_USERAGENT, "KarafutoMapCore/0.0");
            curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
            curl_easy_setopt(curl, CURLOPT_URL, c_url);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mCurlBuffer);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFunc);

            mCurlResult = curl_easy_perform(curl);

            curl_easy_cleanup(curl);
        }

        void onTaskComplete() override {
            auto cache = (*mTarget)[mQuadcode];

            // copy buffer to cache
            auto* buffer = new uint8_t[mCurlBuffer.size()];
            memcpy(buffer, mCurlBuffer.data(), mCurlBuffer.size());
            (*cache)["height"] = buffer;

            std::cout << "Load complete! " << mCurlBuffer.size() << " bytes" << std::endl;
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