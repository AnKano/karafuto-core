#pragma once

#include <queue>
#include <mutex>
#include <optional>

#include "tasks/BaseTask.hpp"

namespace KCore {
    template<class T>
    class Queue {
    private:
        std::queue<std::shared_ptr<T>> mQueue;
        std::mutex mQueueLock;

    public:
        void pushTask(T *task) {
            std::lock_guard<std::mutex> lock{mQueueLock};

            std::shared_ptr<T> task_ptr(task);
            mQueue.push(task_ptr);
        }

        std::shared_ptr<T> popTask() {
            std::lock_guard<std::mutex> lock{mQueueLock};

            if (mQueue.empty())
                return nullptr;

            auto task = mQueue.front();
            mQueue.pop();

            return task;
        }

        void clear() {
            std::lock_guard<std::mutex> lock{mQueueLock};
            std::swap(mQueue, std::queue<std::shared_ptr<T>>());
        }
    };
}