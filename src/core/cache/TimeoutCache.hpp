#pragma once

#include <map>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <iostream>
#include <algorithm>
#include <optional>

using namespace std::chrono;
using namespace std::chrono_literals;

namespace KCore {
    template<class T>
    class TimeoutCache {
    private:
        struct CacheElement {
            T element;
            std::chrono::time_point<system_clock> time;
        };

        std::map<std::string, CacheElement> mCachedElements;
        std::mutex mCacheLock;

        std::chrono::milliseconds mCheckInterval = 1s;
        std::chrono::milliseconds mStayAliveInterval = 5s;

        bool mShouldClose = false;
        bool mReadyToBeDead = false;

        std::shared_ptr<std::thread> mCacheThread;

    public:
        T* operator[](const std::string &key) {
            std::lock_guard<std::mutex> lock{mCacheLock};
            return getByKey(key);
        }

        TimeoutCache() {
            mCacheThread = std::make_shared<std::thread>([this]() {
                runCacheLoop();
            });
            mCacheThread->detach();
        }

        ~TimeoutCache() {
            setShouldClose(true);
            // await to thread stop working
            while (getWorkingStatus());
        }

        bool keyInCache(const std::string &key, const bool &actualize = false) {
            std::lock_guard<std::mutex> lock{mCacheLock};

            auto inCache = mCachedElements.find(key);
            if (inCache == mCachedElements.end())
                return false;

            if (actualize)
                mCachedElements[key] = {
                        mCachedElements[key].element,
                        std::chrono::system_clock::now()
                };

            return true;
        }

        const T &setOrReplace(const std::string &key, const T &&element) {
            std::lock_guard<std::mutex> lock{mCacheLock};

            mCachedElements[key] = {
                    element,
                    std::chrono::system_clock::now()
            };

            return mCachedElements[key].element;
        }

        const T &setOrReplace(const std::string &key, const T &element) {
            std::lock_guard<std::mutex> lock{mCacheLock};

            mCachedElements[key] = {
                    element,
                    std::chrono::system_clock::now()
            };

            return mCachedElements[key].element;
        }

        T* getByKey(const std::string &key) {
            if (mCachedElements.find(key) == mCachedElements.end())
                return nullptr;

            // update time if it's exists
            mCachedElements[key].time = std::chrono::system_clock::now();
            return &mCachedElements[key].element;
        }

        void runCacheLoop() {
            while (!mShouldClose) {
                mCacheLock.lock();

                auto timePoint = system_clock::now();

                for (auto it = mCachedElements.begin(); it != mCachedElements.end();) {
                    auto key = it->first;
                    auto element = it->second;

                    auto timeDelta = duration_cast<milliseconds>(timePoint - element.time);
                    auto stayAliveInMilliseconds = duration_cast<milliseconds>(mStayAliveInterval);
                    if (timeDelta >= stayAliveInMilliseconds) {
                        std::cout << "element with tag \"" + key + "\" expired" << std::endl;
                        it = mCachedElements.erase(it);
                    } else
                        ++it;
                }

                mCacheLock.unlock();

                std::this_thread::sleep_for(mCheckInterval);
            }

            mReadyToBeDead = true;
        }

        void setCheckInterval(const uint64_t &seconds) {
            mCheckInterval = std::chrono::seconds(seconds);
        }

        void setCheckInterval(const std::chrono::seconds &value) {
            mCheckInterval = value;
        }

        void setStayAliveInterval(const uint64_t &seconds) {
            mStayAliveInterval = std::chrono::seconds(seconds);
        }

        void setStayAliveInterval(const std::chrono::seconds &value) {
            mStayAliveInterval = value;
        }

    private:
        void setShouldClose(const bool &value) {
            mShouldClose = value;
        }

        [[nodiscard]]
        bool getWorkingStatus() const {
            return mReadyToBeDead;
        }
    };
}