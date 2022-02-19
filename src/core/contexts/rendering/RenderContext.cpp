#include "RenderContext.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../../MapCore.hpp"

namespace KCore {
    RenderContext::RenderContext() {
        mRenderThread = std::make_unique<std::thread>([this]() {
            if (!glfwInit())
                throw std::runtime_error("Can't instantiate glfw module!");

            // invisible window actually not create any context. it is needed
            // as a root object for OpenGL processes
#ifdef __EMSCRIPTEN__
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#else
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

//            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            mWindowContext_ptr = glfwCreateWindow(256, 256, "Karafuto Core", nullptr, nullptr);
            if (!mWindowContext_ptr)
                throw std::runtime_error("Can't instantiate glfw window!");
            glfwMakeContextCurrent(mWindowContext_ptr);

            glewExperimental = GL_TRUE;

            GLenum err = glewInit();
            if (err != GLEW_OK) {
                glfwTerminate();
                throw std::runtime_error(std::string("Could initialize GLEW, error = ") +
                                         (const char *) glewGetErrorString(err));
            }

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

    RenderContext::~RenderContext() {
        setShouldClose(true);
        // await to thread stop working
        while (getWorkingStatus()) std::this_thread::sleep_for(10ms);
    }

    void RenderContext::setCheckInterval(const uint64_t &value) {
        mCheckInterval = std::chrono::milliseconds(value);
    }

    void RenderContext::setCheckInterval(const std::chrono::milliseconds &value) {
        mCheckInterval = value;
    }

    void RenderContext::setShouldClose(const bool &value) {
        mShouldClose = value;
    }

    bool RenderContext::getWorkingStatus() const {
        return mReadyToBeDead;
    }

    void RenderContext::pushTaskToQueue(RenderTask *task) {
        mQueue.pushTask(task);
    }

    void RenderContext::clearQueue() {
        mQueue.clear();
    }

    void RenderContext::pushTextureDataToGPUQueue(const std::string &basicString,
                                                  const std::shared_ptr<std::vector<uint8_t>> &sharedPtr) {
        std::lock_guard<std::mutex> lock{mTextureQueueLock};
        mTexturesQueue.emplace_back(basicString, sharedPtr);
    }

    void RenderContext::loadEverythingToGPU() {
//        std::lock_guard<std::mutex> lock{mTextureQueueLock};
//
//        for (auto &[key, value]: mTexturesQueue) {
//            auto buffer = value.get();
//            auto texture = std::make_shared<KCore::OpenGL::Texture>();
//            texture->setData(256, 256,
//                             GL_RGB, GL_RGB, GL_UNSIGNED_BYTE,
//                             buffer->data());
//            mGPUTextures[key] = texture;
//
//            value.reset();
//        }
//
//        mTexturesQueue.clear();
    }

    void RenderContext::runRenderLoop() {
        while (!mShouldClose) {
            auto task = mQueue.popTask();
            while (task) {
                loadEverythingToGPU();

                // clear canvas and bind shader
                mFramebuffer->bindAndClear();
                mShader->bind();

                if (!relatedTexturesAvailable(task)) continue;

                auto rootQuadcode = task->mQuadcode;
                glm::mat4 scaleMatrix, translationMatrix;
                for (const auto &item: task->mChilds) {
                    prepareTransformForChild(rootQuadcode, item, scaleMatrix, translationMatrix);
                    drawTileToTexture(item, scaleMatrix, translationMatrix);
                }
                for (const auto &item: task->mParents) {
                    prepareTransformForParent(rootQuadcode, item, scaleMatrix, translationMatrix);
                    drawTileToTexture(item, scaleMatrix, translationMatrix);
                }

                auto buffer = mFramebuffer->getColorAttachTexture()->getTextureData();
                auto buffer_ptr = std::make_shared<std::vector<uint8_t>>(buffer);
//                mCore_ptr->mDataStash.setOrReplace(task->mQuadcode + ".meta.image", buffer_ptr);
//
//                mCore_ptr->pushEventToContentQueue(MapEvent::MakeRenderLoadedEvent(task->mQuadcode));

                // get next task or nullptr
                task = mQueue.popTask();

                glfwPollEvents();
            }

            std::this_thread::sleep_for(mCheckInterval);
        }

        dispose();
        mReadyToBeDead = true;
    }

    void RenderContext::initShader() {
        mShader = std::make_shared<KCore::OpenGL::Shader>();
        mShader->addFragmentShader(KCore::OpenGL::BuiltIn::TextureRenderer::fs, OpenGL::Shader::Text);
        mShader->addVertexShader(KCore::OpenGL::BuiltIn::TextureRenderer::vs, OpenGL::Shader::Text);
        mShader->build();
    }

    void RenderContext::initSquareMesh() {
        OpenGL::MeshDescription meshDescription;

        std::vector<glm::vec3> posData = {{-1.0f, 1.0f,  0.0f},
                                          {1.0f,  1.0f,  0.0f},
                                          {1.0f,  -1.0f, 0.0f},
                                          {-1.0f, -1.0f, 0.0f}};
        std::vector<glm::vec2> uvsData = {{0.0f, 1.0f},
                                          {1.0f, 1.0f},
                                          {1.0f, 0.0f},
                                          {0.0f, 0.0f}};
        std::vector<uint32_t> idxData = {0, 1, 2, 0, 2, 3};

        auto converted_position_data = anythingToByteVector(
                &posData[0],
                posData.size(),
                sizeof(posData[0])
        );
        auto converted_uvs_data = anythingToByteVector(
                &uvsData[0],
                uvsData.size(),
                sizeof(uvsData[0])
        );

        meshDescription.setIndicesBuffer(idxData);
        meshDescription.setShader(mShader.get());
        auto positionsDescriptor = OpenGL::AttributeDescription{
                "a_position", std::nullopt,
                converted_position_data, GL_FLOAT,
                3, sizeof(float)
        };
        auto uvsDescriptor = OpenGL::AttributeDescription{
                "a_uv", std::nullopt,
                converted_uvs_data, GL_FLOAT,
                2, sizeof(float)
        };

        meshDescription.addAttribDescription(positionsDescriptor);
        meshDescription.addAttribDescription(uvsDescriptor);

        mMesh = std::make_shared<OpenGL::Mesh>(meshDescription);
    }

    void RenderContext::initCanvasFramebuffer() {
        //!TODO: remove to parameter
        mFramebuffer = std::make_shared<KCore::OpenGL::ColorFramebuffer>(2048, 2048);
    }

    void RenderContext::dispose() {
        glfwDestroyWindow(mWindowContext_ptr);
        glfwTerminate();
    }

    glm::vec4 RenderContext::getRandomGreyscale() {
        auto r = ((float) rand() / (RAND_MAX));
        auto g = ((float) rand() / (RAND_MAX));
        auto b = ((float) rand() / (RAND_MAX));

        return {r, g, b, 1.0f};
    }

    bool RenderContext::relatedTexturesAvailable(const std::shared_ptr<RenderTask> &task) {
        // check child nodes
        for (const auto &item: task->mChilds)
            if (mGPUTextures.find(item + ".common.image") == std::end(mGPUTextures))
                return false;

        // check parent nodes
        for (const auto &item: task->mParents)
            if (mGPUTextures.find(item + ".common.image") == std::end(mGPUTextures))
                return false;

        return true;
    }

    void RenderContext::prepareTransformForChild(const std::string &rootQuadcode, const std::string &childQuadcode,
                                                 glm::mat4 &scaleMatrix, glm::mat4 &translationMatrix) {
        auto formulae = childQuadcode.substr(rootQuadcode.length());

        auto difference = (float) formulae.size();
        float scale = 1.0f / powf(2.0f, difference);

        float step = 0.5f;
        glm::vec3 position{0.0f};
        for (const auto &in: formulae) {
            if (in == '0') {
                position.x -= step;
                position.y -= step;
            } else if (in == '1') {
                position.x += step;
                position.y -= step;
            } else if (in == '2') {
                position.x -= step;
                position.y += step;
            } else if (in == '3') {
                position.x += step;
                position.y += step;
            }
            step /= 2.0f;
        }

        scaleMatrix = glm::scale(glm::vec3{scale});
        translationMatrix = glm::translate(glm::vec3{position.x, position.y, 0.0f});
    }

    void RenderContext::prepareTransformForParent(const std::string &rootQuadcode, const std::string &parentQuadcode,
                                                  glm::mat4 &scaleMatrix, glm::mat4 &translationMatrix) {
        auto reverseFormulae = rootQuadcode.substr(parentQuadcode.length(),
                                                   rootQuadcode.length() - parentQuadcode.length());

        auto difference = (float) reverseFormulae.size();
        float scale = 1.0f * powf(2.0f, difference);

        float step = 0.5f;
        glm::vec3 position{0.0f};
        for (const auto &in: reverseFormulae) {
            if (in == '0') {
                position.x -= step;
                position.y -= step;
            } else if (in == '1') {
                position.x += step;
                position.y -= step;
            } else if (in == '2') {
                position.x -= step;
                position.y += step;
            } else if (in == '3') {
                position.x += step;
                position.y += step;
            }
            step *= 2.0f;
        }

        scaleMatrix = glm::scale(glm::vec3{scale});
        translationMatrix = glm::translate(glm::vec3{position.x, position.y, 0.0f});
    }

    void RenderContext::drawTileToTexture(const std::string &quadcode,
                                          const glm::mat4 &scaleMatrix, const glm::mat4 &translationMatrix) {
        int32_t u_scale_matrix_position = mShader->uniform_position("u_scale_matrix");
        int32_t u_translation_matrix_position = mShader->uniform_position("u_translation_matrix");
        int32_t u_color = mShader->uniform_position("u_color");
        int32_t u_diffuse = mShader->uniform_position("u_diffuse");

        uint8_t slot = 0;
        mGPUTextures[quadcode + ".common.image"]->bind(slot);
        glUniform1i(u_diffuse, slot);
        glUniform4fv(u_color, 1, glm::value_ptr(getRandomGreyscale()));
        glUniformMatrix4fv(u_scale_matrix_position, 1, GL_FALSE, glm::value_ptr(scaleMatrix));
        glUniformMatrix4fv(u_translation_matrix_position, 1, GL_FALSE, glm::value_ptr(translationMatrix));

        mMesh->draw();
    }
}