#pragma once

#include <string>
#include <functional>
#include <thread>
#include <optional>
#include <mutex>

#include "LRUCache17.hpp"

namespace KCore {
    class INetworkAdapter {
    private:
        lru17::Cache<std::string, std::vector<std::uint8_t>> mNetworkCache{2 << 7, 2 << 4};
        std::mutex mNetworkCacheMutex;

    protected:
        const char *mUserAgent = "KarafutoMapCore/0.1";

    protected:
        void insertToCache(const std::string &key, const std::vector<std::uint8_t> &val) {
            std::lock_guard<std::mutex> lock{mNetworkCacheMutex};

            mNetworkCache.insert(key, val);
        }

        std::optional<std::vector<std::uint8_t>> getFromCache(const std::string &key) {
            std::lock_guard<std::mutex> lock{mNetworkCacheMutex};

            if (mNetworkCache.contains(key))
                return mNetworkCache.get(key);
            else
                return std::nullopt;
        }

    public:
        INetworkAdapter() = default;

        void AsyncGETRequest(const std::string &url,
                             const std::function<void(const std::vector<uint8_t> &)> &callback) {
            AsyncRequest(url, "GET", callback);
        }

        std::vector<uint8_t> SyncGETRequest(const std::string &url, const std::string &method) {
            return SyncRequest(url, "GET");
        }

        void AsyncRequest(const std::string &url, const std::string &method,
                          const std::function<void(const std::vector<uint8_t> &)> &callback) {
            std::thread{[this, url, method, callback]() {
                auto result = SyncRequest(url, method);
                callback(result);
            }}.detach();
        }

        virtual std::vector<uint8_t> SyncRequest(const std::string &url, const std::string &method) = 0;
    };
}