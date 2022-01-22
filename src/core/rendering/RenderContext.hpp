#pragma once

#include <glad/glad.h>

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <thread>
#include <chrono>
#include <iostream>

#include "../queues/BaseQueue.hpp"

using namespace std::chrono_literals;

namespace KCore {
    class RenderContext {
    private:
        GLFWwindow *mWindowContext_ptr;
        BaseQueue mQueue;

        std::chrono::milliseconds mCheckInterval = 1ms;
        bool mShouldClose = false;
        bool mReadyToBeDead = false;

    public:
        RenderContext() {
            if (!glfwInit())
                throw std::runtime_error("Can't instantiate glfw module!");

            // invisible window actually not create any context. it is needed
            // as a root object for OpenGL processes

            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            mWindowContext_ptr = glfwCreateWindow(1, 1, "Karafuto Core", nullptr, nullptr);
            if (!mWindowContext_ptr)
                throw std::runtime_error("Can't instantiate glfw window!");
            glfwMakeContextCurrent(mWindowContext_ptr);

//#ifdef WIN32
            gladLoadGL();
//#endif

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glEnable(GL_BLEND);
            glEnable(GL_STENCIL_TEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        ~RenderContext() {
            setShouldClose(true);

            glfwDestroyWindow(mWindowContext_ptr);
            glfwTerminate();
        }

        void runRenderLoop() {
            while (!mShouldClose) {
                //!TODO: perform queue actions here
                auto task = mQueue.popTask();
                while (task) {
                    task->invoke();
                    task = mQueue.popTask();
                }

                std::this_thread::sleep_for(mCheckInterval);
            }

            mReadyToBeDead = true;
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

        BaseQueue *getQueue() {
            return &mQueue;
        }
    };
}