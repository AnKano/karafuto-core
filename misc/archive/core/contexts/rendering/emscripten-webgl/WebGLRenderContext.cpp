//
// Created by anshu on 3/30/2022.
//

#include "WebGLRenderContext.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <iostream>

#include "../../../misc/Utils.hpp"
#include "../../../geography/tiles/GenericTile.hpp"
#include "../../../worlds/TerrainedWorld.hpp"

namespace KCore::WebGL {
    void WebGLRenderContext::initialize() {
        std::cout << "WebGL Render Thread ID: " << std::this_thread::get_id() << std::endl;

        EmscriptenWebGLContextAttributes attr;
        attr.majorVersion = 2;
        attr.minorVersion = 0;
        emscripten_webgl_init_context_attributes(&attr);
        ctx = emscripten_webgl_create_context("#tileRenderer", &attr);
        emscripten_webgl_make_context_current(ctx);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glEnable(GL_BLEND);
        glEnable(GL_STENCIL_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        initShader();
        initSquareMesh();
        initCanvasFramebuffer();
    }

    void WebGLRenderContext::performLoopStep() {
        if (mWorldAdapter->getAsyncEventsLength() != 0) {
            std::this_thread::sleep_for(350ms);
            return;
        }

        emscripten_webgl_make_context_current(ctx);

        mShader->bind();

        auto state = getCurrentTileState();

        for (const auto &tile: state) {
            mFramebuffer->bindAndClear();

            loadTileTexturesToGPU(tile);

            auto rootQuadcode = tile->getTileDescription().getQuadcode();
            glm::mat4 scaleMatrix, translationMatrix;
            for (const auto &item: tile->getChildQuadcodes()) {
                if (mInGPUTextures.count(item) == 0) continue;
//                std::cout << state.size() << std::endl;
                prepareTransformForChild(rootQuadcode, item, scaleMatrix, translationMatrix);
                drawTileToTexture(item, scaleMatrix, translationMatrix);
            }
            for (const auto &item: tile->getParentQuadcodes()) {
                if (mInGPUTextures.count(item) == 0) continue;
                prepareTransformForParent(rootQuadcode, item, scaleMatrix, translationMatrix);
                drawTileToTexture(item, scaleMatrix, translationMatrix);
            }

            auto buffer = mFramebuffer->getColorAttachTexture()->getTextureData();

            auto rawBuffer = new uint8_t[2048 * 2048 * 4];
            std::copy(buffer.data(), buffer.data() + (2048 * 2048 * 4), rawBuffer);

            mWorldAdapter->pushToAsyncEvents(MapEvent::MakeRenderLoadedEvent(rootQuadcode, rawBuffer));

            unloadTexturesFromGPU();

            emscripten_webgl_commit_frame();
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }

    void WebGLRenderContext::drawTileToTexture(const std::string &quadcode,
                                               const glm::mat4 &scaleMatrix, const glm::mat4 &translationMatrix) {
        int32_t u_scale_matrix_position = mShader->uniform_position("u_scale_matrix");
        int32_t u_translation_matrix_position = mShader->uniform_position("u_translation_matrix");
        int32_t u_color = mShader->uniform_position("u_color");
        int32_t u_diffuse = mShader->uniform_position("u_diffuse");

        auto getRandomGreyscaleColor = []() {
            auto r = ((float) rand() / (RAND_MAX));
            auto g = ((float) rand() / (RAND_MAX));
            auto b = ((float) rand() / (RAND_MAX));
            return glm::vec4{r, g, b, 1.0f};
        };

        uint8_t slot = 0;
        mInGPUTextures[quadcode]->bind(slot);
        glUniform1i(u_diffuse, slot);
        glUniform4fv(u_color, 1, glm::value_ptr(getRandomGreyscaleColor()));
        glUniformMatrix4fv(u_scale_matrix_position, 1, GL_FALSE, glm::value_ptr(scaleMatrix));
        glUniformMatrix4fv(u_translation_matrix_position, 1, GL_FALSE, glm::value_ptr(translationMatrix));

        mMesh->draw();
    }

    void WebGLRenderContext::dispose() {

    }

    void WebGLRenderContext::loadTileTexturesToGPU(GenericTile *tile) {
        for (const auto &item: tile->getChildQuadcodes()) {
            if (mInRAMNotConvertedTextures.count(item) == 0)
                continue;

            auto &raw = mInRAMNotConvertedTextures[item];

            auto image = STBImageUtils::decodeImageBuffer(
                    reinterpret_cast<const uint8_t *>(raw.data()),
                    raw.size(), 0
            );

            auto texture = std::make_shared<KCore::WebGL::Texture>();
            texture->setData(256, 256,
                             GL_RGB, GL_RGB, GL_UNSIGNED_BYTE,
                             image.data());
            mInGPUTextures[item] = texture;
        }

        for (const auto &item: tile->getParentQuadcodes()) {
            if (mInRAMNotConvertedTextures.count(item) == 0) continue;
            auto &raw = mInRAMNotConvertedTextures[item];

            auto image = STBImageUtils::decodeImageBuffer(
                    reinterpret_cast<const uint8_t *>(raw.data()),
                    raw.size(), 0
            );

            auto texture = std::make_shared<KCore::WebGL::Texture>();
            texture->setData(256, 256, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE,
                             raw.data());
            mInGPUTextures[item] = texture;
        }
    }

    void WebGLRenderContext::prepareTransformForChild(
            const std::string &rootQuadcode, const std::string &childQuadcode,
            glm::mat4 &scaleMatrix, glm::mat4 &translationMatrix
    ) {
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

    void WebGLRenderContext::prepareTransformForParent(
            const std::string &rootQuadcode, const std::string &parentQuadcode,
            glm::mat4 &scaleMatrix, glm::mat4 &translationMatrix
    ) {
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

    void WebGLRenderContext::unloadTexturesFromGPU() {
        mInGPUTextures.clear();
    }

    void WebGLRenderContext::initShader() {
        mShader = std::make_shared<KCore::WebGL::Shader>();
        mShader->addFragmentShader(KCore::WebGL::BuiltIn::TextureRenderer::fragmentShaderSource);
        mShader->addVertexShader(KCore::WebGL::BuiltIn::TextureRenderer::vertexShaderSource);
        mShader->build();
    }

    void WebGLRenderContext::initCanvasFramebuffer() {
        mFramebuffer = std::make_shared<KCore::WebGL::ColorFramebuffer>(2048, 2048);
    }

    void WebGLRenderContext::initSquareMesh() {
        WebGL::MeshDescription meshDescription;

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
        auto positionsDescriptor = WebGL::AttributeDescription{
                "a_position", std::nullopt,
                converted_position_data, GL_FLOAT,
                3, sizeof(float)
        };
        auto uvsDescriptor = WebGL::AttributeDescription{
                "a_uv", std::nullopt,
                converted_uvs_data, GL_FLOAT,
                2, sizeof(float)
        };

        meshDescription.addAttribDescription(positionsDescriptor);
        meshDescription.addAttribDescription(uvsDescriptor);

        mMesh = std::make_shared<WebGL::Mesh>(meshDescription);
    }
}