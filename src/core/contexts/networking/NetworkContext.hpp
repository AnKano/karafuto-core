#pragma once

#include <stdexcept>
#include <thread>
#include <chrono>
#include <iostream>

#include "../../queue/Queue.hpp"

using namespace std::chrono_literals;

namespace KCore {
    class NetworkContext {
    private:
        Queue<BaseTask> mQueue;

        std::unique_ptr<std::thread> mRenderThread;

        std::chrono::milliseconds mCheckInterval = 1ms;
        bool mShouldClose = false;
        bool mReadyToBeDead = false;

    public:
        NetworkContext() {
            mRenderThread = std::make_unique<std::thread>([this]() {
                runRenderLoop();
            });
            mRenderThread->detach();
        }

        ~NetworkContext() {
            dispose();
        }

        void setCheckInterval(const uint64_t &value) {
            mCheckInterval = std::chrono::milliseconds(value);
        }

        void setCheckInterval(const std::chrono::milliseconds &value) {
            mCheckInterval = value;
        }

        void setShouldClose(const bool &value) {
            mShouldClose = value;
        }

        [[nodiscard]]
        bool getWorkingStatus() const {
            return mReadyToBeDead;
        }

        void pushTaskToQueue(BaseTask *task) {
            mQueue.pushTask(task);
        }

    private:
        Queue<BaseTask> *getQueue() {
            return &mQueue;
        }

        void runRenderLoop() {
            while (!mShouldClose) {
                auto task = mQueue.popTask();
                while (task) {
                    task->invoke();

                    // get next task or nullptr
                    task = mQueue.popTask();
                }

                std::this_thread::sleep_for(mCheckInterval);
            }

            dispose();
            mReadyToBeDead = true;
        }

        void dispose() {
            setShouldClose(true);
            // await to thread stop working
            while (getWorkingStatus());
        }
    };
}