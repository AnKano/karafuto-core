#pragma once

#include <string>
#include <utility>
#include <iostream>

#include "curl/curl.h"

#include "BaseTask.hpp"

namespace KCore {
    class NetworkTask : public BaseTask {
    private:
        std::string mPayload;

        std::string mCurlBuffer;
        CURLcode mCurlResult;

    public:
        NetworkTask(std::string payload) : mPayload(std::move(payload)) {}

        void performTask() override {
            const char *url = "http://tile.openstreetmap.org/18/156834/92354.png";
            char curlErrorBuffer[CURL_ERROR_SIZE];

            CURL *curl = curl_easy_init();
            if (!curl) return;

            curl_easy_setopt(curl, CURLOPT_USERAGENT, "KarafutoMapCore/0.0");
            curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mCurlBuffer);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFunc);

            mCurlResult = curl_easy_perform(curl);

            curl_easy_cleanup(curl);
        }

        void onTaskComplete() override {
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