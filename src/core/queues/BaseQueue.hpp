#pragma once

#include <queue>
#include <mutex>
#include <optional>

#include "tasks/BaseTask.hpp"

namespace KCore {
    class BaseQueue {
    private:
        std::queue<BaseTask *> mQueue;
        std::mutex mQueueLock;

    public:
        void pushTask(BaseTask *task) {
            std::lock_guard<std::mutex> lock{mQueueLock};
            mQueue.push(task);
        }

        BaseTask *popTask() {
            std::lock_guard<std::mutex> lock{mQueueLock};

            if (mQueue.empty())
                return nullptr;

            auto task = mQueue.front();
            mQueue.pop();

            return task;
        }
    };
}