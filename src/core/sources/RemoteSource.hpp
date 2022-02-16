#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <fstream>
#include <vector>
#include "BaseSource.hpp"
#include "../geography/TileDescription.hpp"

namespace KCore {
    class RemoteSource : public BaseSource {
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
        RemoteSource(std::string prefix, std::string affix)
                : mURLPrefix(std::move(prefix)), mURLSuffix(std::move(affix)) {
            restoreRawUrl();
        }

        uint8_t *getDataForTile(uint8_t zoom, uint16_t x, uint16_t y, uint16_t slicesX, uint16_t slicesY) override {
            return nullptr;
        }

        std::string bakeUrl(const TileDescription &desc) {
            return mURLPrefix + desc.tileURL() + mURLSuffix;
        }

    protected:
        std::vector<std::shared_ptr<BaseSourcePart>> getRelatedPieces(uint8_t zoom, uint16_t x, uint16_t y) override {
            return {};
        }

        void createPartFile(const std::string &path) override {}

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