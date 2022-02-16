#pragma once

#include <functional>
#include <utility>
#include <iostream>

namespace KCore {
    class NetworkRequest {
    private:
        std::string mUrl;

        std::vector<uint8_t> mTransmissionBuffer;

        std::function<void(const std::vector<uint8_t> &)> mOnSuccess = [this](const std::vector<uint8_t> &data) {
            std::cout << mUrl << " loaded " << data.size() << " bytes and flushed to /dev/null" << std::endl;
        };

        std::function<void()> mOnFailure = [this]() {
            std::cout << mUrl << " not loaded" << std::endl;
        };

    public:
        explicit NetworkRequest(std::string url) : mUrl(std::move(url)) {}

        NetworkRequest(std::string url,
                       std::function<void(const std::vector<uint8_t> &)> onSuccess,
                       std::function<void()> onFailure)
                : mUrl(std::move(url)),
                  mOnSuccess(std::move(onSuccess)),
                  mOnFailure(std::move(onFailure)) {}

        void Finalize() {
            if (mOnSuccess != nullptr)
                mOnSuccess(mTransmissionBuffer);
        }

        void Break() {
            if (mOnFailure != nullptr)
                mOnFailure();
        }

        const std::string &getUrl() {
            return mUrl;
        }

        const std::vector<uint8_t> &getBuffer() {
            return mTransmissionBuffer;
        }
    };
}