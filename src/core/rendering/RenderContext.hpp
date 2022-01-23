#pragma once

#include <glad/glad.h>

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <thread>
#include <chrono>
#include <iostream>

#include "../queue/Queue.hpp"

using namespace std::chrono_literals;

namespace KCore {
    class RenderContext {
    private:
        GLFWwindow *mWindowContext_ptr;
        Queue mQueue;

        std::unique_ptr<std::thread> mRenderThread;

        std::chrono::milliseconds mCheckInterval = 1ms;
        bool mShouldClose = false;
        bool mReadyToBeDead = false;

    public:
        RenderContext() {
            mRenderThread = std::make_unique<std::thread>([this]() {
                if (!glfwInit())
                    throw std::runtime_error("Can't instantiate glfw module!");

                // invisible window actually not create any context. it is needed
                // as a root object for OpenGL processes

                glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
                mWindowContext_ptr = glfwCreateWindow(1, 1, "Karafuto Core", nullptr, nullptr);
                if (!mWindowContext_ptr)
                    throw std::runtime_error("Can't instantiate glfw window!");
                glfwMakeContextCurrent(mWindowContext_ptr);

                gladLoadGL();

                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glEnable(GL_BLEND);
                glEnable(GL_STENCIL_TEST);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                runRenderLoop();
            });
            mRenderThread->detach();
        }

        ~RenderContext() {
            setShouldClose(true);
            // await to thread stop working
            while (getWorkingStatus());
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
        Queue *getQueue() {
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
            glfwDestroyWindow(mWindowContext_ptr);
            glfwTerminate();
        }
    };
}