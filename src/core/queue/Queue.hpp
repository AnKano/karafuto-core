#pragma once

#include <queue>
#include <mutex>
#include <optional>

#include "tasks/BaseTask.hpp"

namespace KCore {
    class Queue {
    private:
        std::queue<std::shared_ptr<BaseTask>> mQueue;
        std::mutex mQueueLock;

    public:
        void pushTask(BaseTask *task) {
            std::lock_guard<std::mutex> lock{mQueueLock};

            std::shared_ptr<BaseTask> task_ptr(task);
            mQueue.push(task_ptr);
        }

        std::shared_ptr<BaseTask> popTask() {
            std::lock_guard<std::mutex> lock{mQueueLock};

            if (mQueue.empty())
                return nullptr;

            auto task = mQueue.front();
            mQueue.pop();

            return task;
        }
    };
}