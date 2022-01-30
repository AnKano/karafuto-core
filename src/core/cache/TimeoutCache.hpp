#pragma once

#include "BaseCache.hpp"

namespace KCore {
    template<class T>
    class TimeoutCache : public BaseCache<T> {
    private:
        std::chrono::milliseconds mStayAliveInterval = 5s;

    public:
        TimeoutCache() : BaseCache<T>() {}

        [[maybe_unused]]
        void setInfiniteStayAliveInterval() {
            mStayAliveInterval = std::chrono::milliseconds::max();
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
            for (auto it = this->mCachedElements.begin(); it != this->mCachedElements.end();) {
                auto key = it->first;
                auto element = it->second;

                auto timeDelta = duration_cast<milliseconds>(this->mLastAccessTimePoint - element.time);
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