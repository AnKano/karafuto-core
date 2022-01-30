#pragma once

#include "glad/glad.h"

#define GLFW_INCLUDE_NONE

#include "GLFW/glfw3.h"

#include <stdexcept>
#include <thread>
#include <chrono>
#include <iostream>

#include "../../queue/Queue.hpp"
#include "../../queue/tasks/RenderingTask.hpp"

#include "opengl/Shader.hpp"
#include "opengl/Mesh.hpp"
#include "opengl/BuiltInShaders.inl"
#include "../../misc/Utils.hpp"

using namespace std::chrono_literals;

namespace KCore {
    class RenderContext {
    private:
        GLFWwindow *mWindowContext_ptr;
        std::shared_ptr<KCore::opengl::Shader> mShader;
        std::shared_ptr<KCore::opengl::Mesh> mMesh;
        unsigned int mFramebuffer{};
        unsigned int mTexture{};

        Queue<RenderingTask> mQueue;

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

                initShader();
                initSquareMesh();
                initCanvasFramebuffer();

                runRenderLoop();
            });
            mRenderThread->detach();
        }

        ~RenderContext() {
            setShouldClose(true);
            // await to thread stop working
            while (getWorkingStatus());
        }

        [[maybe_unused]]
        void setCheckInterval(const uint64_t &value) {
            mCheckInterval = std::chrono::milliseconds(value);
        }

        [[maybe_unused]]
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

        void pushTaskToQueue(RenderingTask *task) {
            mQueue.pushTask(task);
        }

    private:
        Queue<RenderingTask> *getQueue() {
            return &mQueue;
        }

        void runRenderLoop() {
            while (!mShouldClose) {
                auto task = mQueue.popTask();
                while (task) {

                    // clear canvas and bind shader
                    glClear(GL_COLOR_BUFFER_BIT);
                    mShader->bind();

                    // render image related to task
                    task->invoke();

                    // swap framebuffer and get events
                    glfwSwapBuffers(mWindowContext_ptr);
                    glfwPollEvents();

                    // get next task or nullptr
                    task = mQueue.popTask();
                }

                std::this_thread::sleep_for(mCheckInterval);
            }

            dispose();
            mReadyToBeDead = true;
        }

        void initShader() {
            mShader = std::make_shared<KCore::opengl::Shader>("Canvas");
            mShader->addFragmentShader(TextureRenderingBuiltin::fragmentShader, opengl::Shader::Text);
            mShader->addVertexShader(TextureRenderingBuiltin::vertexShader, opengl::Shader::Text);
            mShader->build();
        }

        void initSquareMesh() {
            opengl::MeshDescription meshDescription;

            std::vector<glm::vec3> posData = {{-1.0f, 1.0f,  0.0f},
                                              {1.0f,  1.0f,  0.0f},
                                              {1.0f,  -1.0f, 0.0f},
                                              {-1.0f, -1.0f, 0.0f}};
            std::vector<uint32_t> idxData = {0, 1, 2, 0, 2, 3};

            auto converted_position_data = anythingToByteVector(
                    &posData[0],
                    posData.size(),
                    sizeof(posData[0])
            );

            meshDescription.setIndicesBuffer(idxData);
            meshDescription.setShader(mShader.get());
            meshDescription.addAttribDescription({"a_position",
                                                  std::nullopt,
                                                  converted_position_data,
                                                  GL_FLOAT,
                                                  3,
                                                  sizeof(float)});

            mMesh = std::make_shared<opengl::Mesh>(meshDescription);
        }

        void initCanvasFramebuffer() {
            glGenTextures(1, &mTexture);
            glBindTexture(GL_TEXTURE_2D, mTexture);

            uint8_t data[512 * 512 * 3];
            for (std::size_t i = 0; i < 512 * 512 * 3; i++)
                data[i] = i;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

            glGenFramebuffers(1, &mFramebuffer);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);

            glEnable(GL_BLEND);
            glEnable(GL_STENCIL_TEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        void dispose() {
            glfwDestroyWindow(mWindowContext_ptr);
            glfwTerminate();
        }
    };
}