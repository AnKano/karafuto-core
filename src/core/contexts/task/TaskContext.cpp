#include "TaskContext.hpp"

#include "../../MapCore.hpp"

namespace KCore {
    TaskContext::TaskContext() {
        std::cout << "Main Thread ID: " << std::this_thread::get_id() << std::endl;
        mThread = std::make_unique<std::thread>([this] {
            std::cout << "Task Thread ID: " << std::this_thread::get_id() << std::endl;
            runTaskLoop();
        });
        mThread->detach();
    }

    TaskContext::~TaskContext() {
        dispose();
    }

    void TaskContext::pushTaskToQueue(CallbackTask *task) {
        mTaskQueue.pushTask(task);
    }

    void TaskContext::runTaskLoop() {
        while (!mShouldClose) {
            auto task = mTaskQueue.popTask();
            while (task) {
                task->invoke();

                // get next task or nullptr
                task = mTaskQueue.popTask();
            }

            std::this_thread::sleep_for(mCheckInterval);
        }

        dispose();
        mReadyToBeDead = true;
    }

    void TaskContext::dispose() {
        setShouldClose(true);
        // await to thread stop working
        while (getWorkingStatus()) std::this_thread::sleep_for(10ms);
    }

    void TaskContext::initialize() {

    }

    void TaskContext::performLoopStep() {

    }
}