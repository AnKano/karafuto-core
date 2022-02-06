#include "TaskContext.hpp"

#include "../../MapCore.hpp"

namespace KCore {
    TaskContext::TaskContext(MapCore *core) {
        mTaskThread = std::make_unique<std::thread>([this]() {
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
            auto task = mTaskQueue.popTask();
            while (task) {
                task->invoke();

                // get next task or nullptr
                task = mTaskQueue.popTask();

                glfwPollEvents();
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