#pragma once

#include <chrono>
#include <thread>

#include "../../queue/Queue.hpp"
#include "../../queue/tasks/CallbackTask.hpp"
#include "../IContext.hpp"

using namespace std::chrono_literals;

namespace KCore {
    class MapCore;

    class TaskContext : public IContext {
    private:
        Queue<CallbackTask> mTaskQueue;

    public:
        TaskContext();

        ~TaskContext();

        void pushTaskToQueue(CallbackTask *task);

        void runTaskLoop();

    private:
        void dispose() override;

        void initialize() override;

        void performLoopStep() override;
    };
}