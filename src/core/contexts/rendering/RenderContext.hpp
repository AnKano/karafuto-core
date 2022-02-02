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
    class MapCore;

    class RenderContext {
    private:
        GLFWwindow *mWindowContext_ptr;
        MapCore* mCore_ptr;

        std::shared_ptr<KCore::opengl::Shader> mShader;
        std::shared_ptr<KCore::opengl::Mesh> mMesh;
        unsigned int mFramebuffer{};
        unsigned int mTexture{};

        Queue<RenderingTask> mQueue;

        std::map<std::string, unsigned int> mTextures;
        std::vector<std::pair<std::string, std::shared_ptr<std::vector<uint8_t>>>> mTexturesQueue;
        std::mutex mQueueLock;

        std::unique_ptr<std::thread> mRenderThread;

        std::chrono::milliseconds mCheckInterval = 1ms;
        bool mShouldClose = false;
        bool mReadyToBeDead = false;

    public:
        RenderContext(MapCore *core, BaseCache<std::shared_ptr<void>> *stash);

        ~RenderContext();

        [[maybe_unused]]
        void setCheckInterval(const uint64_t &value);

        [[maybe_unused]]
        void setCheckInterval(const std::chrono::milliseconds &value);

        void setShouldClose(const bool &value);

        [[nodiscard]]
        bool getWorkingStatus() const;

        void pushTaskToQueue(RenderingTask *task);

        void pushTextureDataToGPUQueue(const std::string &basicString,
                                       const std::shared_ptr<std::vector<uint8_t>> &sharedPtr);

        void LoadEverythingToGPU();

        void clearQueue();

    private:
        Queue<RenderingTask> *getQueue();

        void runRenderLoop();

        void initShader();

        void initSquareMesh();

        void initCanvasFramebuffer();

        void dispose();
    };
}