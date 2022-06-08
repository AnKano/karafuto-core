#pragma once

#include <thread>

using namespace std::chrono_literals;

namespace KCore {
    class IContext {
    protected:
        // thread specific variables
        std::unique_ptr<std::thread> mThread;

        std::chrono::milliseconds mCheckInterval = 1s;
        bool mShouldClose = false;
        bool mReadyToBeDead = false;

        virtual ~IContext();

        [[maybe_unused]]
        void setCheckInterval(const uint64_t &value);

        [[maybe_unused]]
        void setCheckInterval(const std::chrono::milliseconds &value);

        void setShouldClose(const bool &value);

        [[nodiscard]]
        bool getWorkingStatus() const;

        virtual void initialize() = 0;

        virtual void performLoopStep() = 0;

        virtual void dispose() = 0;
    };
}