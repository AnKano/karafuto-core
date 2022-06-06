#pragma once

#include <GL/glew.h>
#include <memory>

#include "Texture.hpp"

namespace KCore::WebGL {
    class ColorFramebuffer {
    private:
        GLuint mId{0};
        GLsizei mHeight, mWidth;

        std::shared_ptr<KCore::WebGL::Texture> mColorAttach;

    public:
        ColorFramebuffer(const GLsizei &width, const GLsizei &height) :
                mWidth(width), mHeight(height) {
            glGenFramebuffers(1, &mId);
            attachColorTexture(width, height);
        }

        ~ColorFramebuffer() {
            glDeleteFramebuffers(1, &mId);
        }

        void bindAndClear() const {
            glViewport(0, 0, mWidth, mHeight);
            glBindFramebuffer(GL_FRAMEBUFFER, mId);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        static void unbind() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        KCore::WebGL::Texture *getColorAttachTexture() {
            return mColorAttach.get();
        }

    private:
        void attachColorTexture(const GLsizei &width, const GLsizei &height) {
            mColorAttach = std::make_shared<KCore::WebGL::Texture>();
            mColorAttach->setData(width, height, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

            glBindFramebuffer(GL_FRAMEBUFFER, mId);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorAttach->getId(), 0);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    };
}