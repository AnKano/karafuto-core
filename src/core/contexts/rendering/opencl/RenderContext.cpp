//
// Created by Anton Shubin on 3/15/22.
//

#include "RenderContext.hpp"

#include <iostream>

#include "../../../worlds/BaseWorld.hpp"

#include <gzip/compress.hpp>

namespace KCore::OpenCL {
    void RenderContext::initialize() {
        int err;                            // error code returned from api calls
        cl_device_id device_id;             // compute device id

        // Connect to a compute device
        err = clGetDeviceIDs(nullptr, CL_DEVICE_TYPE_GPU, 1, &device_id, nullptr);
        if (err != CL_SUCCESS) printf("Error: Failed to create a device group!\n");

        // Create a compute context
        mContext = clCreateContext(nullptr, 1, &device_id, nullptr, nullptr, &err);
        if (!mContext) printf("Error: Failed to create a compute context!\n");

        // Create a command commands
        mCommands = clCreateCommandQueue(mContext, device_id, 0, &err);
        if (!mCommands) printf("Error: Failed to create a command commands!\n");

        // Create the compute program from the source buffer
        const char *KernelSource = KCore::OpenCL::BuiltIn::TextureRenderer::kernel.c_str();
        mProgram = clCreateProgramWithSource(mContext, 1, (const char **) &KernelSource, nullptr, &err);
        if (!mProgram) printf("Error: Failed to create compute program!\n");

        // Build the program executable
        err = clBuildProgram(mProgram, 0, nullptr, nullptr, nullptr, nullptr);
        if (err != CL_SUCCESS) {
            size_t len;
            char buffer[2048];

            printf("Error: Failed to build program executable!\n");
            clGetProgramBuildInfo(mProgram, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
            printf("%s\n", buffer);
            exit(1);
        }

        for (int i = 0; i < 2; i++) {
            mOutBuffer[i] = clCreateBuffer(mContext, CL_MEM_WRITE_ONLY, mOutImageBytes, nullptr, &err);
            if (!mOutBuffer[i]) {
                printf("Error: Failed to create image!\n");
                exit(1);
            }
        }
    }

    void RenderContext::childTransform(const std::string &rootQuadcode, const std::string &childQuadcode,
                                       unsigned int &offsetX, unsigned int &offsetY, unsigned int &depth) {
        auto formulae = childQuadcode.substr(rootQuadcode.length());

        auto difference = (float) formulae.size();

        float step = mOutImageWidth * 0.5f;
        glm::vec3 position{0.0f};
        for (const auto &in: formulae) {
            if (in == '0') {
                position.x += 0;
                position.y += 0;
            } else if (in == '1') {
                position.x += step;
                position.y += 0;
            } else if (in == '2') {
                position.x += 0;
                position.y += step;
            } else if (in == '3') {
                position.x += step;
                position.y += step;
            }
            step /= 2.0f;
        }

        offsetX = position.x;
        offsetY = position.y;

        depth = difference;
    }

    void RenderContext::runRenderLoop() {
        auto bufferIdx = 0;

        while (!mShouldClose) {
            auto metas = getCurrentTileState();
            for (const auto &meta: metas) {

                for (const auto &item: meta->getChildQuadcodes()) {
                    if (mInRAMNotConvertedTextures.count(item) == 0) continue;

                    unsigned int offsetX, offsetY, depth;
                    auto rootQuadcode = meta->getTileDescription().getQuadcode();
                    childTransform(rootQuadcode, item, offsetX, offsetY, depth);

                    auto& data = mInRAMNotConvertedTextures[item];

                    Tile tile(&mContext, 256, 256, data);
                    tile.setup(mOutImageWidth, mOutImageHeight, offsetX, offsetY, depth);

                    performRenderKernel(tile, bufferIdx);

                    tile.dispose();
                }

                clFinish(mCommands);

                auto results = std::vector<uint8_t>();
                results.resize(mOutImageBytes);

                int err = clEnqueueReadBuffer(mCommands, mOutBuffer[bufferIdx], CL_FALSE, 0,
                                              mOutImageBytes, results.data(), 0, nullptr, nullptr);
                if (err != CL_SUCCESS) {
                    printf("Error: Failed to read output array! %d\n", err);
                    exit(1);
                }

                performWipeKernel(bufferIdx);

                std::string compressed_data = gzip::compress(
                        reinterpret_cast<const char *>(results.data()), mOutImageBytes, Z_BEST_SPEED
                );
                std::cout << mOutImageBytes << " to " << compressed_data.size() << std::endl;

                auto rawBuffer = new std::vector<uint8_t>{};
                rawBuffer->resize(mOutImageBytes);
                std::copy(compressed_data.begin(), compressed_data.end(), rawBuffer->data());

                auto rootQuadcode = meta->getTileDescription().getQuadcode();
                mWorldAdapter->pushToAsyncEvents(MapEvent::MakeRenderLoadedEvent(rootQuadcode, rawBuffer));

                std::this_thread::sleep_for(100ms);
            }
        }

        dispose();
        mReadyToBeDead = true;
    }

    void RenderContext::setupRenderKernel() {
        int err = 0;

        // Create the compute kernel in the program we wish to run
        mRenderKernel = clCreateKernel(mProgram, "oneShotRenderToRGB565", &err);
        if (!mRenderKernel || err != CL_SUCCESS) {
            printf("Error: Failed to create compute kernel!\n");
            exit(1);
        }

        renderKernelSetup = true;
    }

    void RenderContext::performRenderKernel(const Tile &tile, unsigned int bufferIdx) {
        if (!renderKernelSetup) setupRenderKernel();

        int err = 0;
        err |= clSetKernelArg(mRenderKernel, 0, sizeof(cl_mem), &tile.getImageMem());
        err |= clSetKernelArg(mRenderKernel, 1, sizeof(cl_mem), &mOutBuffer[bufferIdx]);
        err |= clSetKernelArg(mRenderKernel, 2, sizeof(unsigned int), &tile.mPosX);
        err |= clSetKernelArg(mRenderKernel, 3, sizeof(unsigned int), &tile.mPosY);
        err |= clSetKernelArg(mRenderKernel, 4, sizeof(unsigned int), &tile.mTargetX);
        err |= clSetKernelArg(mRenderKernel, 5, sizeof(unsigned int), &tile.mTargetY);
        if (err != CL_SUCCESS) {
            printf("Error: Failed to set kernel arguments! %d\n", err);
            exit(1);
        }

        size_t global[3] = {tile.mTargetX, tile.mTargetY};
        size_t local[3] = {1, 1};

        err = clEnqueueNDRangeKernel(mCommands, mRenderKernel, 2, nullptr, global, local, 0, nullptr, nullptr);
        if (err) {
            printf("Error: Failed to execute kernel!\n");
            exit(1);
        }
    }

    void RenderContext::setupWipeKernel() {
        int err = 0;

        // Create the compute kernel in the program we wish to run
        mWipeKernel = clCreateKernel(mProgram, "wipeCanvas", &err);
        if (!mWipeKernel || err != CL_SUCCESS) {
            printf("Error: Failed to create compute kernel!\n");
            exit(1);
        }

        wipeKernelSetup = true;
    }

    void RenderContext::performWipeKernel(unsigned int bufferIdx) {
        if (!wipeKernelSetup) setupWipeKernel();

        size_t global[3] = {mOutImageWidth, mOutImageHeight};
        size_t local[3] = {1, 1};

        int err = 0;
        err |= clSetKernelArg(mWipeKernel, 0, sizeof(cl_mem), &mOutBuffer[bufferIdx]);
        if (err != CL_SUCCESS) {
            printf("Error: Failed to set kernel arguments! %d\n", err);
            exit(1);
        }

        err = clEnqueueNDRangeKernel(mCommands, mWipeKernel, 2, nullptr, global, local, 0, nullptr, nullptr);
        if (err) {
            printf("Error: Failed to execute kernel!\n");
            exit(1);
        }

        clFinish(mCommands);
    }

    void RenderContext::dispose() {
        clReleaseMemObject(mOutBuffer[0]);
        clReleaseMemObject(mOutBuffer[1]);

        clReleaseProgram(mProgram);
        clReleaseKernel(mRenderKernel);
        clReleaseCommandQueue(mCommands);
        clReleaseContext(mContext);
    }
}
