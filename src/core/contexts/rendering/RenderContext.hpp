#pragma once

#include <GL/glew.h>

#if defined(_MSC_VER)
    #include <Windows.h>
#endif

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <thread>
#include <chrono>
#include <iostream>

#include "../../misc/Utils.hpp"

#include "../../queue/Queue.hpp"
#include "../../queue/tasks/RenderTask.hpp"

#include "opengl/Shader.hpp"
#include "opengl/Mesh.hpp"
#include "opengl/Texture.hpp"
#include "opengl/ColorFramebuffer.hpp"
#include "opengl/misc/BuiltInShaders.inl"

using namespace std::chrono_literals;

namespace KCore {
    class MapCore;

    class RenderContext {
    private:
        GLFWwindow *mWindowContext_ptr;
        MapCore *mCore_ptr;

        std::shared_ptr<KCore::OpenGL::Shader> mShader;
        std::shared_ptr<KCore::OpenGL::Mesh> mMesh;
        std::shared_ptr<KCore::OpenGL::ColorFramebuffer> mFramebuffer;

        Queue<RenderTask> mQueue;

        std::map<std::string, std::shared_ptr<KCore::OpenGL::Texture>> mGPUTextures;
        std::vector<std::pair<std::string, std::shared_ptr<std::vector<uint8_t>>>> mTexturesQueue;
        std::mutex mTextureQueueLock;

        std::unique_ptr<std::thread> mRenderThread;

        std::chrono::milliseconds mCheckInterval = 1ms;
        bool mShouldClose = false;
        bool mReadyToBeDead = false;

    public:
        RenderContext(MapCore *core);

        ~RenderContext();

        [[maybe_unused]]
        void setCheckInterval(const uint64_t &value);

        [[maybe_unused]]
        void setCheckInterval(const std::chrono::milliseconds &value);

        void setShouldClose(const bool &value);

        [[nodiscard]]
        bool getWorkingStatus() const;

        void pushTaskToQueue(RenderTask *task);

        void pushTextureDataToGPUQueue(const std::string &basicString,
                                       const std::shared_ptr<std::vector<uint8_t>> &sharedPtr);

        void loadEverythingToGPU();

        void clearQueue();

    private:
        void runRenderLoop();

        void initShader();

        void initSquareMesh();

        void initCanvasFramebuffer();

        void dispose();

        static glm::vec4 getRandomGreyscale();

        bool relatedTexturesAvailable(const std::shared_ptr<RenderTask> &task);

        static void prepareTransformForChild(
                const std::string &rootQuadcode, const std::string &childQuadcode,
                glm::mat4 &scaleMatrix, glm::mat4 &translationMatrix
        );

        static void prepareTransformForParent(
                const std::string &rootQuadcode, const std::string &parentQuadcode,
                glm::mat4 &scaleMatrix, glm::mat4 &translationMatrix
        );

        void drawTileToTexture(
                const std::string &quadcode,
                const glm::mat4 &scaleMatrix, const glm::mat4 &translationMatrix
        );
    };
}