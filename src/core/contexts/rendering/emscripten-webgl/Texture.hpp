#pragma once

#include <GL/glew.h>

#include <vector>
#include <iostream>

namespace KCore::WebGL {
    class Texture {
    private:
        GLuint mId{0};
        GLsizei mWidth{0}, mHeight{0};

    public:
        Texture() {
            glGenTextures(1, &mId);
            glBindTexture(GL_TEXTURE_2D, mId);

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            setWrapParameter(GL_CLAMP_TO_EDGE);
            setFilterParameter(GL_LINEAR);
        }

        ~Texture() {
            glDeleteTextures(1, &mId);
        }

        void setData(const GLsizei &width, const GLsizei &height,
                     const GLint &internalformat, const GLenum &format, const GLenum &type,
                     void *data) {
            mWidth = width;
            mHeight = height;

            glBindTexture(GL_TEXTURE_2D, mId);

            glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, type, data);

            glBindTexture(GL_TEXTURE_2D, 0);
        }

        [[nodiscard]]
        std::vector<uint8_t> getTextureData() const {
            const unsigned int bytesPerPixel = 4;

            std::vector<uint8_t> buffer;
            buffer.resize(mHeight * mWidth * bytesPerPixel);

            glBindTexture(GL_TEXTURE_2D, mId);
//            glActiveTexture(GL_TEXTURE0);
//            glBindTexture(GL_TEXTURE_2D, mId);
//            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
            glReadPixels(0, 0, mWidth, mHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());

//            for (int i = 0; i < 10; i++) {
//                std::cout << buffer[i] << " ";
//            }
//            std::cout << std::endl;

//            glBindTexture(GL_TEXTURE_2D, 0);

            return buffer;
        }

        [[maybe_unused]]
        void setFilterParameter(const GLint &type) const {
            glBindTexture(GL_TEXTURE_2D, mId);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, type);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, type);

            glBindTexture(GL_TEXTURE_2D, 0);
        }

        [[maybe_unused]]
        void setWrapParameter(const GLint &type) const {
            glBindTexture(GL_TEXTURE_2D, mId);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, type);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, type);

            glBindTexture(GL_TEXTURE_2D, 0);
        }

        [[nodiscard]]
        const GLuint &getId() const {
            return mId;
        }

        void bind(unsigned int unit) const {
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, mId);
        }

        void bind() const {
            bind(0);
        }

        static void unbind() {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    };
}