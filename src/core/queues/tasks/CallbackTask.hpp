#pragma once

#include "BaseTask.hpp"

#include <functional>
#include <utility>

namespace KCore {
    class CallbackTask : public BaseTask {
    private:
        std::function<void()> onTaskBeforeLambda;
        std::function<void()> onTaskPerformLambda;
        std::function<void()> onTaskCompleteLambda;

    public:
        CallbackTask(std::function<void()> task,
                     std::function<void()> before = nullptr,
                     std::function<void()> after = nullptr) :
                onTaskPerformLambda(std::move(task)),
                onTaskBeforeLambda(std::move(before)),
                onTaskCompleteLambda(std::move(after)) {}

        void onTaskBeforeStart() override {
            if (onTaskCompleteLambda) onTaskBeforeLambda();
        }

        void performTask() override {
            if (onTaskPerformLambda) onTaskPerformLambda();
        }

        void onTaskComplete() override {
            if (onTaskCompleteLambda) onTaskCompleteLambda();
        }
    };
}