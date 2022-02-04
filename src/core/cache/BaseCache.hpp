#pragma once

#include <map>
#include <string>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include <memory>
#include <iostream>
#include <algorithm>
#include <optional>

using namespace std::chrono;
using namespace std::chrono_literals;

namespace KCore {
    typedef std::chrono::time_point<high_resolution_clock> TimePoint;

    template<class T>
    class BaseCache {
    protected:
        struct CacheElement {
            T element;
            TimePoint time;
        };

        std::map<std::string, std::vector<CacheElement>> mCachedElements;

        TimePoint mLastAccessTimePoint;

        std::mutex mCacheAccessLock;
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

            mLastAccessTimePoint = high_resolution_clock::now();
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
            std::lock_guard<std::mutex> lock{mCacheAccessLock};

            mLastAccessTimePoint = high_resolution_clock::now();

            auto inCache = mCachedElements.find(key) != mCachedElements.end();
            if (!inCache)
                return false;

            if (actualize)
                mCachedElements[key].back() = {
                        mCachedElements[key].element,
                        std::chrono::high_resolution_clock::now()
                };

            return true;
        }

        virtual const T &setOrReplace(const std::string &key, const T &&element) {
            std::lock_guard<std::mutex> lock{mCacheAccessLock};
            mLastAccessTimePoint = high_resolution_clock::now();

            if (mCachedElements.count(key) == 0)
                mCachedElements[key] = {};

            mCachedElements[key].push_back({
                element,
                high_resolution_clock::now()
            });

            return mCachedElements[key].back().element;
        }

        virtual const T &setOrReplace(const std::string &key, const T &element) {
            std::lock_guard<std::mutex> lock{mCacheAccessLock};
            mLastAccessTimePoint = high_resolution_clock::now();

            if (mCachedElements.count(key) == 0)
                mCachedElements[key] = {};

            mCachedElements[key].push_back({
                element,
                high_resolution_clock::now()
            });

            return mCachedElements[key].back().element;
        }

        T *getByKey(const std::string &key) {
            std::lock_guard<std::mutex> lock{mCacheAccessLock};
            mLastAccessTimePoint = high_resolution_clock::now();

            if (mCachedElements.count(key) == 0)
                return nullptr;

            // update time if it's exists
            mCachedElements[key].back().time = high_resolution_clock::now();
            return &mCachedElements[key].back().element;
        }

        void globalLock() {
            mCacheAccessLock.lock();
        }

        void globalUnlock() {
            mCacheAccessLock.unlock();
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
            std::lock_guard<std::mutex> lock{mCacheAccessLock};
            mCachedElements.clear();
        }

    protected:
        virtual void inLoopCheck() = 0;

    private:
        void runCacheLoop() {
            while (!mShouldClose) {
                this->mCacheAccessLock.lock();
                inLoopCheck();
                this->mCacheAccessLock.unlock();

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