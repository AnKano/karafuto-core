#pragma once

#include <chrono>
#include <thread>

#include "../../queue/Queue.hpp"
#include "../../queue/tasks/CallbackTask.hpp"

using namespace std::chrono_literals;

namespace KCore {
    class MapCore;

    class TaskContext {
    private:
        Queue<CallbackTask> mTaskQueue;

        std::unique_ptr<std::thread> mTaskThread;
        std::chrono::milliseconds mWaitInterval = 10ms;
        bool mShouldClose = false;
        bool mReadyToBeDead = false;

    public:
        TaskContext();

        ~TaskContext();

        [[maybe_unused]]
        void setWaitInterval(const uint64_t &value);

        [[maybe_unused]]
        void setWaitInterval(const std::chrono::milliseconds &value);

        void setShouldClose(const bool &value);

        [[nodiscard]]
        bool getWorkingStatus() const;

        void pushTaskToQueue(CallbackTask *task);

        void runTaskLoop();

    private:
        void dispose();
    };
}