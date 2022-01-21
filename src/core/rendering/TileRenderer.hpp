#pragma once

#include <glad/glad.h>

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>

#include "opengl/Shader.hpp"
#include "opengl/Mesh.hpp"
#include "Misc.cpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/transform.hpp"

namespace KCore {
    class TileRenderer {
    private:
        GLFWwindow *mOffscreenContext;

        std::shared_ptr<KCore::opengl::Shader> mShader;
        std::shared_ptr<KCore::opengl::Mesh> mMesh;

        unsigned int mFramebuffer{};
        unsigned int mTexture{};
    public:
        TileRenderer() {
            if (!glfwInit())
                throw std::runtime_error("Can't instantiate glfw module!");

            // invisible window actually not create any context. it is needed
            // as a root object for opengl processes
            //glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            mOffscreenContext = glfwCreateWindow(512, 512, "", nullptr, nullptr);
            glfwMakeContextCurrent(mOffscreenContext);

            gladLoadGL();

            // set black clear color
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

            mShader = std::make_shared<KCore::opengl::Shader>("TileRenderer");
            mShader->addShaderPairFromPath("assets/shaders/plane");
            mShader->build();

            opengl::MeshDescription meshDescription;
            {
                std::vector<glm::vec3> posData = {{-1.0f, 1.0f,  0.0f},
                                                  {1.0f,  1.0f,  0.0f},
                                                  {1.0f,  -1.0f, 0.0f},
                                                  {-1.0f, -1.0f, 0.0f}};
                std::vector<uint32_t> idxData = {0, 1, 2, 0, 2, 3};
                auto converted_position_data = ConvertToBytes(&posData[0], posData.size(), sizeof(posData[0]));

                meshDescription.setIndicesBuffer(idxData);
                meshDescription.setShader(mShader.get());
                meshDescription.addAttribDescription(
                        {"a_position", std::nullopt, converted_position_data, GL_FLOAT, 3, sizeof(float)});
            }

            mMesh = std::make_shared<opengl::Mesh>(meshDescription);

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

        ~TileRenderer() {
            glfwTerminate();
        }

        void loadToTexture() {
            glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

            while (!glfwWindowShouldClose(mOffscreenContext)) {
                glClear(GL_COLOR_BUFFER_BIT);

                mShader->bind();
                int32_t uScaleMatrixPos = mShader->uniform_position("u_scale_matrix");
                int32_t uTranslationMatrixPos = mShader->uniform_position("u_translation_matrix");
                int32_t uColorPos = mShader->uniform_position("u_color");

                glm::mat4 translation = glm::translate(glm::vec3{0.0f});
                glm::mat4 scale = glm::scale(glm::vec3{1.5f});

                glUniformMatrix4fv(uScaleMatrixPos, 1, GL_FALSE, glm::value_ptr(scale));
                glUniformMatrix4fv(uTranslationMatrixPos, 1, GL_FALSE, glm::value_ptr(translation));
                glUniform4fv(uColorPos, 1,
                             glm::value_ptr(glm::vec4{135.0f / 255.0f, 206.0f / 255.0f, 250.0f / 255.0f, 1.0f}));

                mMesh->draw();

                translation = glm::translate(glm::vec3{0.0f});
                scale = glm::scale(glm::vec3{0.5f});

                glUniformMatrix4fv(uScaleMatrixPos, 1, GL_FALSE, glm::value_ptr(scale));
                glUniformMatrix4fv(uTranslationMatrixPos, 1, GL_FALSE, glm::value_ptr(translation));
                glUniform4fv(uColorPos, 1,
                             glm::value_ptr(glm::vec4{255.0f / 255.0f, 105.0f / 255.0f, 180.0f / 255.0f, 1.0f}));

                mMesh->draw();

                translation = glm::translate(glm::vec3{0.0f});
                scale = glm::scale(glm::vec3{0.45f});

                glUniformMatrix4fv(uScaleMatrixPos, 1, GL_FALSE, glm::value_ptr(scale));
                glUniformMatrix4fv(uTranslationMatrixPos, 1, GL_FALSE, glm::value_ptr(translation));
                glUniform4fv(uColorPos, 1,
                             glm::value_ptr(glm::vec4{255.0f / 255.0f, 182.0f / 255.0f, 193.0f / 255.0f, 1.0f}));

                mMesh->draw();


                glfwSwapBuffers(mOffscreenContext);
                glfwPollEvents();
            }
        }

        void readFromTexture() {
            glBindTexture(GL_TEXTURE_2D, mTexture);

            uint8_t data[512 * 512 * 3];

            auto start = std::chrono::system_clock::now();
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            auto stop = std::chrono::system_clock::now();

            auto elapsed = stop - start;
            std::cout << elapsed / std::chrono::microseconds(1) << " for 512x512x3 reading" << std::endl;
        }
    };
}