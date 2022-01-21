#pragma once

#include <string>
#include "curl/curl.h"

namespace KCore {
    class NetworkQueue {
    public:
        NetworkQueue() = default;

        std::string curlBuffer;

        static size_t curlWriteFunc(char *data, size_t size, size_t nmemb, std::string *buffer) {
            size_t result = 0;

            if (buffer != nullptr) {
                buffer->append(data, size * nmemb);
                result = size * nmemb;
            }
            return result;
        }

        void perform_request() {
            const char *url = "http://tile.openstreetmap.org/18/156834/92354.png";
            char curlErrorBuffer[CURL_ERROR_SIZE];

            CURL *curl = curl_easy_init();
            if (!curl) return;

            curl_easy_setopt(curl, CURLOPT_USERAGENT, "KarafutoMapCore/0.0");
            curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlBuffer);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFunc);

            CURLcode curlResult = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            if (curlResult == CURLE_OK) {
                curl = curl;
            } else {
                curl = curl;
            }
        }
    };
}