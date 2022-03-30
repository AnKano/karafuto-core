#pragma once

#include "../IRenderContext.hpp"

#include "Shader.hpp"
#include "Mesh.hpp"
#include "ColorFramebuffer.hpp"

#include "misc/BuiltInShaders.inl"

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include <glm/glm.hpp>

namespace KCore::WebGL {
    class WebGLRenderContext : public IRenderContext {
        std::shared_ptr<KCore::WebGL::Shader> mShader;
        std::shared_ptr<KCore::WebGL::Mesh> mMesh;
        std::shared_ptr<KCore::WebGL::ColorFramebuffer> mFramebuffer;

        std::map<std::string, std::shared_ptr<KCore::WebGL::Texture>> mInGPUTextures;

        EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;

    public:
        WebGLRenderContext(TerrainedWorld *world) : IRenderContext(world) {}

    private:
        void initShader();

        void initSquareMesh();

        void initCanvasFramebuffer();

        void loadTileTexturesToGPU(GenericTile *tile);

        void unloadTexturesFromGPU();

        static void prepareTransformForChild(const std::string &rootQuadcode,
                                      const std::string &childQuadcode,
                                      glm::mat4 &scaleMatrix,
                                      glm::mat4 &translationMatrix);

        void drawTileToTexture(const std::string &quadcode,
                               const glm::mat4 &scaleMatrix,
                               const glm::mat4 &translationMatrix);

    protected:
        virtual void initialize();

        virtual void performLoopStep();

        virtual void dispose();
    };
}
