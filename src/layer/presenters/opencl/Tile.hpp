#pragma once

#include <vector>
#include <cmath>
#include <iostream>
#include <cstdint>

#include "../../../misc/STBImageUtils.hpp"

namespace KCore::OpenCL {
    class Tile {
    public:
        std::size_t mTextureHeight, mTextureWidth;

        unsigned int mPosX, mPosY;
        unsigned int mTargetX, mTargetY;

        std::vector<uint8_t> mData;

        cl_context *mContext;
        cl_mem mImageMem;

        Tile(cl_context *context, const std::size_t &height, const std::size_t &width)
                : mTextureHeight(height), mTextureWidth(width) {
            mContext = context;

            mData.resize(height * width * 4);

            float step = 256.0f / (float) height;

            int pixel = 0;
            for (auto j = 0u; j < height; ++j) {
                for (auto i = 0u; i < width; ++i) {
                    mData[pixel * 4] = std::ceil(i * step);
                    mData[pixel * 4 + 1] = std::ceil(j * step);
                    mData[pixel * 4 + 2] = std::ceil(j * step);
                    mData[pixel * 4 + 3] = 0xFF;

                    pixel++;
                }
            }

            loadToGPU();
        }

        Tile(cl_context *context, const std::size_t &height, const std::size_t &width, const std::vector<uint8_t> &data)
                : mTextureHeight(height), mTextureWidth(width) {
            mContext = context;
            mData = data;
            loadToGPU();
        }

        void dispose() {
            clReleaseMemObject(mImageMem);
        }

        [[nodiscard]]
        const cl_mem &getImageMem() const {
            return mImageMem;
        }

        void setup(unsigned int screenWidth, unsigned int screenHeight,
                   unsigned int positionX, unsigned int positionY, unsigned int depth) {
            mPosX = positionX;
            mPosY = positionY;

            mTargetX = screenWidth / std::pow(2.0, depth);
            mTargetY = screenHeight / std::pow(2.0, depth);
        }

    private:
        void loadToGPU() {
            auto format = cl_image_format{CL_RGBA, CL_UNSIGNED_INT8};

            auto *convert = new uint8_t[mTextureWidth * mTextureHeight * 4];
            int pixel = 0;
            for (auto j = 0u; j < mTextureHeight; ++j) {
                for (auto i = 0u; i < mTextureWidth; ++i) {
                    convert[pixel * 4] = mData[pixel * 3];
                    convert[pixel * 4 + 1] = mData[pixel * 3 + 1];
                    convert[pixel * 4 + 2] = mData[pixel * 3 + 2];
                    convert[pixel * 4 + 3] = 0xFF;

                    pixel++;
                }
            }

            int err;
            mImageMem = clCreateImage2D(
                    *mContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &format,
                    mTextureWidth, mTextureHeight, 0, convert, &err
            );
            if (!mImageMem) {
                printf("Error: Failed to load image!\n");
                exit(1);
            }

            delete[] convert;
        }
    };
}