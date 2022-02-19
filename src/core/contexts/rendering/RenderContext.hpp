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
    class BaseWorld;

    class GenericTile;

    class RenderContext {
    private:
        BaseWorld *mWorldAdapter;
        GLFWwindow *mWindowContext_ptr;

        std::shared_ptr<KCore::OpenGL::Shader> mShader;
        std::shared_ptr<KCore::OpenGL::Mesh> mMesh;
        std::shared_ptr<KCore::OpenGL::ColorFramebuffer> mFramebuffer;

        std::mutex mTexturesLock, mTileStateLock;
        std::vector<KCore::GenericTile *> mCurrentTileState;
        std::map<std::string, std::vector<uint8_t>> mInRAMNotConvertedTextures;
        std::map<std::string, std::shared_ptr<KCore::OpenGL::Texture>> mInGPUTextures;

        std::unique_ptr<std::thread> mRenderThread;

        std::chrono::milliseconds mCheckInterval = 1ms;
        bool mShouldClose = false;
        bool mReadyToBeDead = false;

    public:
        RenderContext(BaseWorld *world);

        ~RenderContext();

        [[maybe_unused]]
        void setCheckInterval(const uint64_t &value);

        [[maybe_unused]]
        void setCheckInterval(const std::chrono::milliseconds &value);

        void setShouldClose(const bool &value);

        [[nodiscard]]
        bool getWorkingStatus() const;

        void storeTextureInContext(const std::vector<uint8_t> &data, const std::string &quadcode);

        void setCurrentTileState(const std::vector<KCore::GenericTile *> &tiles);

    private:
        void runRenderLoop();

        void initShader();

        void initSquareMesh();

        void initCanvasFramebuffer();

        void dispose();

        static glm::vec4 getRandomGreyscaleColor();

        const std::vector<KCore::GenericTile *> &getCurrentTileState();
//        bool relatedTexturesAvailable(const std::shared_ptr<RenderTask> &task);

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

        void loadTileTexturesToGPU(GenericTile *tile);

        void unloadTexturesFromGPU();
    };
}