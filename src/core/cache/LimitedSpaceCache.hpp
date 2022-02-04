#pragma once

#include "BaseCache.hpp"

#include <limits>

namespace KCore {
    template<class T>
    class LimitedSpaceCache : public BaseCache<T> {
    private:
        std::chrono::milliseconds mStayAliveInterval = 20s;
        uint64_t mMaximalCount = 5000;

    public:
        LimitedSpaceCache() : BaseCache<T>() {}

        [[maybe_unused]]
        void setMaximalCount(const uint64_t &count) {
            mMaximalCount = count;
        }

        [[maybe_unused]]
        void setStayAliveInterval(const uint64_t &seconds) {
            mStayAliveInterval = std::chrono::seconds(seconds);
        }

        [[maybe_unused]]
        void setStayAliveInterval(const std::chrono::seconds &value) {
            mStayAliveInterval = value;
        }

    private:
        void inLoopCheck() {
            if (this->mCachedElements.size() < mMaximalCount) return;

            for (auto it = this->mCachedElements.begin(); it != this->mCachedElements.end();) {
                if (this->mCachedElements.size() <= mMaximalCount)
                    break;

                auto key = it->first;
                auto &vecOfElements = it->second;

                it->second = {vecOfElements.back()};
                auto timeDelta = duration_cast<milliseconds>(this->mLastAccessTimePoint - vecOfElements[0].time);
                auto stayAliveInMilliseconds = duration_cast<milliseconds>(mStayAliveInterval);
                if (timeDelta >= stayAliveInMilliseconds) {
                    std::cout << "element with tag \"" + key + "\" expired" << std::endl;
                    it = this->mCachedElements.erase(it);
                } else
                    ++it;
            }
        }
    };
}