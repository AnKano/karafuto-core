#pragma once

#include <memory>

#include "../../../cache/BaseCache.hpp"
#include "../opengl/Texture.hpp"

namespace KCore {
    class TextureCache : public BaseCache<std::shared_ptr<KCore::OpenGL::Texture>> {
    private:
        std::vector<std::string> mKeysToCleanUp;
        std::chrono::milliseconds mStayAliveInterval = 5s;

    public:
        void clearUp() {
            std::lock_guard<std::mutex> lock{mCacheAccessLock};

            for (const auto &key: mKeysToCleanUp) {
                auto &block = mCachedElements[key];
                block = {block.back()};

                auto timeDelta = duration_cast<milliseconds>(this->mLastAccessTimePoint - block[0].time);
                auto stayAliveInMilliseconds = duration_cast<milliseconds>(mStayAliveInterval);
                if (timeDelta >= stayAliveInMilliseconds)
                    mCachedElements.erase(key);
            }
            mKeysToCleanUp.clear();
        }

        void fullClearUp() {
            std::lock_guard<std::mutex> lock{mCacheAccessLock};

            mCachedElements.clear();
        }

    private:
        void inLoopCheck() override {
            for (auto &[key, value]: this->mCachedElements) {
                auto timeDelta = duration_cast<milliseconds>(this->mLastAccessTimePoint - value[0].time);
                auto stayAliveInMilliseconds = duration_cast<milliseconds>(mStayAliveInterval);
                if (timeDelta >= stayAliveInMilliseconds)
                    mKeysToCleanUp.push_back(key);
            }
        }
    };
}