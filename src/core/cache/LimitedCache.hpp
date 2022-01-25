#pragma once

#include "BaseCache.hpp"

namespace KCore {
    template<class T>
    class LimitedCache : public BaseCache<T> {
    private:
        std::chrono::milliseconds mStayAliveInterval = 20s;
        uint64_t mMaximalCount = 5000;

    public:
        LimitedCache() : BaseCache<T>() {}

        void setMaximalCount(const uint64_t &count) {
            mMaximalCount = count;
        }

        void setInfiniteStayAliveInterval() {
            mStayAliveInterval = std::chrono::milliseconds::max();
        }

        void setStayAliveInterval(const uint64_t &seconds) {
            mStayAliveInterval = std::chrono::seconds(seconds);
        }

        void setStayAliveInterval(const std::chrono::seconds &value) {
            mStayAliveInterval = value;
        }

    private:
        void inLoopCheck() {
            if (this->mCachedElements.size() < mMaximalCount) return;

            auto timePoint = system_clock::now();

            for (auto it = this->mCachedElements.begin(); it != this->mCachedElements.end();) {
                if (this->mCachedElements.size() <= mMaximalCount)
                    break;

                auto key = it->first;
                auto element = it->second;

                auto timeDelta = duration_cast<milliseconds>(timePoint - element.time);
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