#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <fstream>
#include <vector>

#include "../../queues/network/NetworkQueue.hpp"

namespace KCore {
    class BaseRemoteSource {
    protected:
        std::string mRawUrl;
        std::string mURLPrefix, mURLSuffix;

        NetworkQueue *mQueue;

    public:
        /** example: https://10.0.0.1:8080/{z}/{x}/{y}.png
         * mURLPrefix - "https://10.0.0.1:8080/"
         * mURLSuffix - ".png"
         **/
        BaseRemoteSource(std::string rawUrl) : mRawUrl(std::move(rawUrl)) {
            restoreAffixes();
        }

        /** example prefix: "https://10.0.0.1:8080/"
         * example affix: ".png"
         * mRawUrl - "https://10.0.0.1:8080/{z}/{x}/{y}.png"
         **/
        BaseRemoteSource(std::string prefix, std::string affix) : mURLPrefix(std::move(prefix)),
                                                                  mURLSuffix(std::move(affix)) {
            restoreRawUrl();
        }

        virtual uint8_t *getTileData(uint8_t zoom, uint16_t x, uint16_t y) = 0;

    private:
        void restoreRawUrl() {
            mRawUrl = mURLPrefix + "{z}/{x}/{y}" + mURLSuffix;
        }

        void restoreAffixes() {
            auto payloadLength = std::string("{z}/{x}/{y}").length();
            auto position = mRawUrl.find("{z}/{x}/{y}");

            mURLPrefix = mRawUrl.substr(0, position);
            mURLSuffix = mRawUrl.substr(position + payloadLength);
        }
    };
}