#pragma once

#include <map>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <memory>
#include <iostream>
#include <algorithm>
#include <optional>

using namespace std::chrono;
using namespace std::chrono_literals;

namespace KCore {
    template<class T>
    class BaseCache {
    protected:
        struct CacheElement {
            T element;
            std::chrono::time_point<system_clock> time;
        };

        std::map<std::string, CacheElement> mCachedElements;

        std::chrono::time_point<system_clock> mLastAccessTimePoint;

        std::mutex mCacheLock;
        std::shared_ptr<std::thread> mCacheThread;
        std::chrono::milliseconds mCheckInterval = 1s;

        bool mShouldClose = false;
        bool mReadyToBeDead = false;

    public:
        BaseCache() {
            mCacheThread = std::make_shared<std::thread>([this]() {
                runCacheLoop();
            });
            mCacheThread->detach();

            mLastAccessTimePoint = system_clock::now();
        }

        ~BaseCache() {
            setShouldClose(true);
            // await to thread stop working
            while (getWorkingStatus());
        }

        T *operator[](const std::string &key) {
            return getByKey(key);
        }

        bool keyInCache(const std::string &key, const bool &actualize = false) {
            std::lock_guard<std::mutex> lock{mCacheLock};

            mLastAccessTimePoint = system_clock::now();

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

        virtual const T &setOrReplace(const std::string &key, const T &&element) {
            std::lock_guard<std::mutex> lock{mCacheLock};
            mLastAccessTimePoint = system_clock::now();

            mCachedElements[key] = {
                    element,
                    std::chrono::system_clock::now()
            };

            return mCachedElements[key].element;
        }

        virtual const T &setOrReplace(const std::string &key, const T &element) {
            std::lock_guard<std::mutex> lock{mCacheLock};
            mLastAccessTimePoint = system_clock::now();

            mCachedElements[key] = {
                    element,
                    std::chrono::system_clock::now()
            };

            return mCachedElements[key].element;
        }

        T *getByKey(const std::string &key) {
            std::lock_guard<std::mutex> lock{mCacheLock};
            mLastAccessTimePoint = system_clock::now();

            if (mCachedElements.find(key) == mCachedElements.end())
                return nullptr;

            // update time if it's exists
            mCachedElements[key].time = std::chrono::system_clock::now();
            return &mCachedElements[key].element;
        }

        void globalLock() {
            mCacheLock.lock();
        }

        void globalUnlock() {
            mCacheLock.unlock();
        }

        [[maybe_unused]]
        void setCheckInterval(const uint64_t &seconds) {
            mCheckInterval = std::chrono::seconds(seconds);
        }

        [[maybe_unused]]
        void setCheckInterval(const std::chrono::seconds &value) {
            mCheckInterval = value;
        }

        void forceClear() {
            std::lock_guard<std::mutex> lock{mCacheLock};
            mCachedElements.clear();
        }

    protected:
        virtual void inLoopCheck() = 0;

    private:
        void runCacheLoop() {
            while (!mShouldClose) {
                mCacheLock.lock();
                inLoopCheck();
                mCacheLock.unlock();

                std::this_thread::sleep_for(mCheckInterval);
            }

            mReadyToBeDead = true;
        }

        void setShouldClose(const bool &value) {
            mShouldClose = value;
        }

        [[nodiscard]]
        bool getWorkingStatus() const {
            return mReadyToBeDead;
        }
    };
}