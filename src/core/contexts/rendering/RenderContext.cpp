#include "RenderContext.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../../MapCore.hpp"

namespace KCore {
    RenderContext::RenderContext(MapCore *core, BaseCache<std::shared_ptr<void>> *stash) : mCore_ptr(core) {
        mRenderThread = std::make_unique<std::thread>([this]() {
            if (!glfwInit())
                throw std::runtime_error("Can't instantiate glfw module!");

            // invisible window actually not create any context. it is needed
            // as a root object for OpenGL processes

//            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            mWindowContext_ptr = glfwCreateWindow(256, 256, "Karafuto Core", nullptr, nullptr);
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

    void RenderContext::pushTaskToQueue(RenderingTask *task) {
        mQueue.pushTask(task);
    }

    Queue<RenderingTask> *RenderContext::getQueue() {
        return &mQueue;
    }


    void RenderContext::clearQueue() {
        mQueue.clear();
    }

    void RenderContext::pushTextureDataToGPUQueue(const std::string &basicString,
                                                  const std::shared_ptr<std::vector<uint8_t>> &sharedPtr) {
        std::lock_guard<std::mutex> lock{mQueueLock};
        mTexturesQueue.emplace_back(basicString, sharedPtr);
    }

    void RenderContext::LoadEverythingToGPU() {
        std::lock_guard<std::mutex> lock{mQueueLock};

        for (const auto &[key, value]: mTexturesQueue) {
            unsigned int textureId;
            glGenTextures(1, &textureId);
            glBindTexture(GL_TEXTURE_2D, textureId);

            mTextures[key] = textureId;

            auto data = value.get();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, data->data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glBindTexture(GL_TEXTURE_2D, 0);
        }

        mTexturesQueue.clear();
    }

    void RenderContext::runRenderLoop() {
        while (!mShouldClose) {
            auto task = mQueue.popTask();
            while (task) {
                LoadEverythingToGPU();

                // clear canvas and bind shader
                glClear(GL_COLOR_BUFFER_BIT);
                glViewport(0, 0, 2048, 2048);
                mShader->bind();

                // check child existence
                auto pass = true;
                for (const auto &item: task->mChilds)
                    if (mTextures.find(item + ".common.image") == std::end(mTextures))
                        pass = false;
                for (const auto &item: task->mParents)
                    if (mTextures.find(item + ".common.image") == std::end(mTextures))
                        pass = false;

                if (pass == false) continue;

                auto quadcode = task->mQuadcode;
                for (const auto &item: task->mChilds) {
                    auto formulae = item.substr(quadcode.length());

                    auto difference = (float) formulae.size();
                    float scale = 1.0f / powf(2.0f, difference);
                    auto scale_matrix = glm::scale(glm::vec3{scale});

                    float step = 0.5f;
                    glm::vec3 position{0.0f};
                    for (const auto &in: formulae) {
                        if (in == '0') {
                            position.x -= step;
                            position.y += step;
                        } else if (in == '1') {
                            position.x += step;
                            position.y += step;
                        } else if (in == '2') {
                            position.x -= step;
                            position.y -= step;
                        } else if (in == '3') {
                            position.x += step;
                            position.y -= step;
                        }
                        step /= 2.0f;
                    }

                    auto translation_matrix = glm::translate(glm::vec3{position.x, -position.y, 0.0f});

                    int32_t u_scale_matrix_position = mShader->uniform_position("u_scale_matrix");
                    int32_t u_translation_matrix_position = mShader->uniform_position("u_translation_matrix");
                    int32_t u_color = mShader->uniform_position("u_color");
                    int32_t u_diffuse = mShader->uniform_position("u_diffuse");

                    glUniformMatrix4fv(u_scale_matrix_position, 1, GL_FALSE, glm::value_ptr(scale_matrix));
                    glUniformMatrix4fv(u_translation_matrix_position, 1, GL_FALSE, glm::value_ptr(translation_matrix));

                    auto r = ((float) rand() / (RAND_MAX));
                    auto g = ((float) rand() / (RAND_MAX));
                    auto b = ((float) rand() / (RAND_MAX));

                    glUniform4fv(u_color, 1, glm::value_ptr(glm::vec4{r, g, b, 1.0f}));

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, mTextures[item + ".common.image"]);
                    glUniform1i(u_diffuse, 0);

                    mMesh->draw();
                }

                for (const auto &item: task->mParents) {
                    auto reverseFormulae = quadcode.substr(item.length(), quadcode.length() - item.length());

                    auto difference = (float) reverseFormulae.size();
                    float scale = 1.0f * powf(2.0f, difference);
                    auto scale_matrix = glm::scale(glm::vec3{scale});

                    float step = 0.5f;
                    glm::vec3 position{0.0f};
                    for (const auto &in: reverseFormulae) {
                        if (in == '0') {
                            position.x -= step;
                            position.y += step;
                        } else if (in == '1') {
                            position.x += step;
                            position.y += step;
                        } else if (in == '2') {
                            position.x -= step;
                            position.y -= step;
                        } else if (in == '3') {
                            position.x += step;
                            position.y -= step;
                        }
                        step *= 2.0f;
                    }

                    auto translation_matrix = glm::translate(glm::vec3{position.x, -position.y, 0.0f});

                    int32_t u_scale_matrix_position = mShader->uniform_position("u_scale_matrix");
                    int32_t u_translation_matrix_position = mShader->uniform_position("u_translation_matrix");
                    int32_t u_color = mShader->uniform_position("u_color");
                    int32_t u_diffuse = mShader->uniform_position("u_diffuse");

                    glUniformMatrix4fv(u_scale_matrix_position, 1, GL_FALSE, glm::value_ptr(scale_matrix));
                    glUniformMatrix4fv(u_translation_matrix_position, 1, GL_FALSE, glm::value_ptr(translation_matrix));

                    auto r = ((float) rand() / (RAND_MAX));
                    auto g = ((float) rand() / (RAND_MAX));
                    auto b = ((float) rand() / (RAND_MAX));

                    glUniform4fv(u_color, 1, glm::value_ptr(glm::vec4{r, g, b, 1.0f}));

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, mTextures[item + ".common.image"]);
                    glUniform1i(u_diffuse, 0);

                    mMesh->draw();
                }

                // render image related to task
//                task->invoke();

                auto textureBytesCount = 2048 * 2048 * 3;
                auto buffer = std::make_shared<std::vector<uint8_t>>();
                buffer->resize(textureBytesCount);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mTexture);
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer->data());
//                if (!mCore_ptr->mDataStash.getByKey(task->mQuadcode + ".meta.image")) {
                mCore_ptr->mDataStash.setOrReplace(task->mQuadcode + ".meta.image",
                                                   std::static_pointer_cast<void>(buffer));

                MapEvent event{};
                event.Type = ContentLoadedRender;
                strcpy_s(event.Quadcode, task->mQuadcode.c_str());
                event.OptionalPayload = nullptr;

                mCore_ptr->pushEventToContentQueue(event);

                std::cout << task->mQuadcode << " loading from renderer complete" << std::endl;
//                }

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

    void RenderContext::initShader() {
        mShader = std::make_shared<KCore::opengl::Shader>("Canvas");
        mShader->addFragmentShader(TextureRenderingBuiltin::fragmentShader, opengl::Shader::Text);
        mShader->addVertexShader(TextureRenderingBuiltin::vertexShader, opengl::Shader::Text);
        mShader->build();
    }

    void RenderContext::initSquareMesh() {
        opengl::MeshDescription meshDescription;

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
        meshDescription.addAttribDescription({"a_position",
                                              std::nullopt,
                                              converted_position_data,
                                              GL_FLOAT,
                                              3,
                                              sizeof(float)});
        meshDescription.addAttribDescription({"a_uv",
                                              std::nullopt,
                                              converted_uvs_data,
                                              GL_FLOAT,
                                              2,
                                              sizeof(float)});

        mMesh = std::make_shared<opengl::Mesh>(meshDescription);
    }

    void RenderContext::initCanvasFramebuffer() {
        glGenTextures(1, &mTexture);
        glBindTexture(GL_TEXTURE_2D, mTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 2048, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenFramebuffers(1, &mFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);

        glEnable(GL_BLEND);
        glEnable(GL_STENCIL_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void RenderContext::dispose() {
        glfwDestroyWindow(mWindowContext_ptr);
        glfwTerminate();
    }
}