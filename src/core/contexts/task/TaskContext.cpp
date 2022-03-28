#include "TaskContext.hpp"

#include "../../MapCore.hpp"

namespace KCore {
    TaskContext::TaskContext() {
        std::cout << "Main Thread ID: " << std::this_thread::get_id() << std::endl;
        mTaskThread = std::make_unique<std::thread>([this] {
            std::cout << "Task Thread ID: " << std::this_thread::get_id() << std::endl;
            runTaskLoop();
        });
        mTaskThread->detach();
    }

    TaskContext::~TaskContext() {
        dispose();
    }

    void TaskContext::setWaitInterval(const uint64_t &value) {
        mWaitInterval = std::chrono::milliseconds(value);
    }

    void TaskContext::setWaitInterval(const std::chrono::milliseconds &value) {
        mWaitInterval = value;
    }

    void TaskContext::setShouldClose(const bool &value) {
        mShouldClose = value;
    }

    bool TaskContext::getWorkingStatus() const {
        return mReadyToBeDead;
    }

    void TaskContext::pushTaskToQueue(CallbackTask *task) {
        mTaskQueue.pushTask(task);
    }

    void TaskContext::runTaskLoop() {
        while (!mShouldClose) {
//            std::cout << "Task ping!" << std::endl;
            auto task = mTaskQueue.popTask();
            while (task) {
                task->invoke();

                // get next task or nullptr
                task = mTaskQueue.popTask();
            }

            std::this_thread::sleep_for(mWaitInterval);
        }

        dispose();
        mReadyToBeDead = true;
    }

    void TaskContext::dispose() {
        setShouldClose(true);
        // await to thread stop working
        while (getWorkingStatus()) std::this_thread::sleep_for(10ms);
    }
}