#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <fstream>
#include <vector>

#include "BaseSourcePart.hpp"

namespace KCore {
    class RemoteSource : public BaseSourcePart {
    protected:
        std::string mRawUrl;
        std::string mURLPrefix, mURLSuffix;

    public:
        /** example: https://10.0.0.1:8080/{z}/{x}/{y}.png
         * mURLPrefix - "https://10.0.0.1:8080/"
         * mURLSuffix - ".png"
         **/
        RemoteSource(std::string rawUrl) : mRawUrl(std::move(rawUrl)) {
            restoreAffixes();
        }

        /** example prefix: "https://10.0.0.1:8080/"
         * example affix: ".png"
         * mRawUrl - "https://10.0.0.1:8080/{z}/{x}/{y}.png"
         **/
        RemoteSource(std::string prefix, std::string affix) : mURLPrefix(std::move(prefix)),
                                                              mURLSuffix(std::move(affix)) {
            restoreRawUrl();
        }

        uint8_t *getTileData(uint8_t zoom, uint16_t x, uint16_t y) override {
            return nullptr;
        }

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