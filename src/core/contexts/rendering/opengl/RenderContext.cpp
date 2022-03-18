#include "RenderContext.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../../MapCore.hpp"

namespace KCore {
    RenderContext::RenderContext(BaseWorld *world) {
        mWorldAdapter = world;

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

        mRenderThread = std::make_unique<std::thread>([this]() {
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

    void RenderContext::runRenderLoop() {
        while (!mShouldClose) {

            // clear canvas and bind shader
            mShader->bind();

            auto state = getCurrentTileState();

            for (const auto &tile: state) {
                mFramebuffer->bindAndClear();

                loadTileTexturesToGPU(tile);

                auto rootQuadcode = tile->getTileDescription().getQuadcode();
                glm::mat4 scaleMatrix, translationMatrix;
                for (const auto &item: tile->getChildQuadcodes()) {
                    if (mInGPUTextures.count(item) == 0) continue;
                    prepareTransformForChild(rootQuadcode, item, scaleMatrix, translationMatrix);
                    drawTileToTexture(item, scaleMatrix, translationMatrix);
                }
                for (const auto &item: tile->getParentQuadcodes()) {
                    if (mInGPUTextures.count(item) == 0) continue;
                    prepareTransformForParent(rootQuadcode, item, scaleMatrix, translationMatrix);
                    drawTileToTexture(item, scaleMatrix, translationMatrix);
                }

                auto buffer = mFramebuffer->getColorAttachTexture()->getTextureData();

                auto rawBuffer = new uint8_t[buffer.size()];
                std::copy(buffer.begin(), buffer.end(), rawBuffer);

                mWorldAdapter->pushToAsyncEvents(MapEvent::MakeRenderLoadedEvent(rootQuadcode, rawBuffer));

                unloadTexturesFromGPU();
            }

//            glfwPollEvents();

            std::this_thread::sleep_for(mCheckInterval);
        }

        dispose();
        mReadyToBeDead = true;
    }

    void RenderContext::loadTileTexturesToGPU(GenericTile *tile) {
        for (const auto &item: tile->getChildQuadcodes()) {
            if (mInRAMNotConvertedTextures.count(item) == 0) {
                continue;
            }
            auto &raw = mInRAMNotConvertedTextures[item];

            auto texture = std::make_shared<KCore::OpenGL::Texture>();
            texture->setData(256, 256, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE,
                             raw.data());
            mInGPUTextures[item] = texture;
        }

        for (const auto &item: tile->getParentQuadcodes()) {
            if (mInRAMNotConvertedTextures.count(item) == 0) continue;
            auto &raw = mInRAMNotConvertedTextures[item];

            auto texture = std::make_shared<KCore::OpenGL::Texture>();
            texture->setData(256, 256, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE,
                             raw.data());
            mInGPUTextures[item] = texture;
        }
    }

    void RenderContext::unloadTexturesFromGPU() {
        mInGPUTextures.clear();
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

    glm::vec4 RenderContext::getRandomGreyscaleColor() {
        auto r = ((float) rand() / (RAND_MAX));
        auto g = ((float) rand() / (RAND_MAX));
        auto b = ((float) rand() / (RAND_MAX));

        return {r, g, b, 1.0f};
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
        mInGPUTextures[quadcode]->bind(slot);
        glUniform1i(u_diffuse, slot);
        glUniform4fv(u_color, 1, glm::value_ptr(getRandomGreyscaleColor()));
        glUniformMatrix4fv(u_scale_matrix_position, 1, GL_FALSE, glm::value_ptr(scaleMatrix));
        glUniformMatrix4fv(u_translation_matrix_position, 1, GL_FALSE, glm::value_ptr(translationMatrix));

        mMesh->draw();
    }

    void RenderContext::storeTextureInContext(const std::vector<uint8_t> &data, const std::string &quadcode) {
        std::lock_guard<std::mutex> lock{mTexturesLock};
        mInRAMNotConvertedTextures[quadcode] = data;
    }

    void RenderContext::setCurrentTileState(const std::vector<KCore::GenericTile *> &tiles) {
        std::lock_guard<std::mutex> lock{mTileStateLock};
        mCurrentTileState = tiles;
    }

    const std::vector<KCore::GenericTile *> &RenderContext::getCurrentTileState() {
        std::lock_guard<std::mutex> lock{mTileStateLock};
        return mCurrentTileState;
    }
}