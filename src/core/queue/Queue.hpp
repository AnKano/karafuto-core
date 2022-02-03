#pragma once

#include <deque>
#include <mutex>
#include <optional>
#include <memory>

#include "tasks/BaseTask.hpp"

namespace KCore {
    template<class T>
    class Queue {
    public:
        std::deque<std::shared_ptr<T>> mQueue;
    private:
        std::mutex mQueueLock;

    public:
        void pushTask(T *task) {
            std::lock_guard<std::mutex> lock{mQueueLock};

            std::shared_ptr<T> task_ptr(task);
            mQueue.push_back(task_ptr);
        }

        std::shared_ptr<T> popTask() {
            std::lock_guard<std::mutex> lock{mQueueLock};

            if (mQueue.empty())
                return nullptr;

            auto task = mQueue.front();
            mQueue.pop_front();

            return task;
        }

        void clear() {
            std::lock_guard<std::mutex> lock{mQueueLock};
            mQueue.clear();
        }
    };
}