//
// Created by Anton Shubin on 3/15/22.
//

#include "OpenCLRenderContext.hpp"

#include <chrono>
#include <thread>

#include "../../Layer.hpp"

namespace KCore::OpenCL {
    void OpenCLRenderContext::initialize() {
        int err;
        cl_uint platforms_num;
        cl_platform_id *platform_id;
        cl_device_id device_id;

        clGetPlatformIDs(0, nullptr, &platforms_num);
        platform_id = (cl_platform_id *) malloc(sizeof(cl_platform_id) * platforms_num);

        clGetPlatformIDs(platforms_num, platform_id, nullptr);

        // Connect to a GPU device
        err = clGetDeviceIDs(platform_id[0], CL_DEVICE_TYPE_GPU, 1, &device_id, nullptr);
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

        mOutBuffer = clCreateBuffer(mContext, CL_MEM_WRITE_ONLY, mOutImageBytes, nullptr, &err);
        if (!mOutBuffer) {
            printf("Error: Failed to create image!\n");
            exit(1);
        }
    }

    void OpenCLRenderContext::childTransform(const std::string &rootQuadcode, const std::string &childQuadcode,
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

    void OpenCLRenderContext::performLoopStep() {
        auto tiles = getCurrentTileState();

        for (auto &meta: tiles) {
            auto items = meta.getRelatedQuadcodes();
            for (const auto &item: items) {
                if (!mCachedTextures.contains(item)) continue;

                unsigned int offsetX, offsetY, depth;
                auto rootQuadcode = meta.getQuadcode();
                childTransform(rootQuadcode, item, offsetX, offsetY, depth);

                auto &data = mCachedTextures.get(item);

                int width = -1, height = -1, channels = -1;
                auto image = STBImageUtils::decodeImageBuffer(data.data(), data.size(), width, height, channels);

                Tile tile(&mContext, width, height, image);
                tile.setup(mOutImageWidth, mOutImageHeight, offsetX, offsetY, depth);

                performRenderKernel(tile);

                tile.dispose();
            }

            clFinish(mCommands);

            auto results = std::vector<uint8_t>();
            results.resize(mOutImageBytes);

            int err = clEnqueueReadBuffer(
                    mCommands, mOutBuffer, CL_FALSE, 0,
                    mOutImageBytes, results.data(), 0, nullptr, nullptr
            );
            if (err != CL_SUCCESS) {
                printf("Error: Failed to read output array! %d\n", err);
                exit(1);
            }

            performWipeKernel();

            std::thread([this, results, meta]() {
                auto image = new ImageResult{mOutImageWidth, mOutImageHeight, mOutImagePixelBytes, results};
                auto rootQuadcode = meta.getQuadcode();
                mWorld->pushToImageEvents(LayerEvent::MakeImageEvent(rootQuadcode, image));
            }).detach();

            std::this_thread::sleep_for(10ms);
        }
    }

    void OpenCLRenderContext::setupRenderKernel() {
        int err = 0;

        // Create the compute kernel in the program we wish to run
        mRenderKernel = clCreateKernel(mProgram, "oneShotRenderToRGB565", &err);
        if (!mRenderKernel || err != CL_SUCCESS) {
            printf("Error: Failed to create compute kernel!\n");
            exit(1);
        }

        renderKernelSetup = true;
    }

    void OpenCLRenderContext::performRenderKernel(const Tile &tile) {
        if (!renderKernelSetup) setupRenderKernel();

        int err = 0;
        err |= clSetKernelArg(mRenderKernel, 0, sizeof(cl_mem), &tile.getImageMem());
        err |= clSetKernelArg(mRenderKernel, 1, sizeof(cl_mem), &mOutBuffer);
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

    void OpenCLRenderContext::setupWipeKernel() {
        int err = 0;

        // Create the compute kernel in the program we wish to run
        mWipeKernel = clCreateKernel(mProgram, "wipeCanvas", &err);
        if (!mWipeKernel || err != CL_SUCCESS) {
            printf("Error: Failed to create compute kernel!\n");
            exit(1);
        }

        wipeKernelSetup = true;
    }

    void OpenCLRenderContext::performWipeKernel() {
        if (!wipeKernelSetup) setupWipeKernel();

        size_t global[3] = {static_cast<size_t>(mOutImageWidth), static_cast<size_t>(mOutImageHeight)};
        size_t local[3] = {1, 1};

        int err = 0;
        err |= clSetKernelArg(mWipeKernel, 0, sizeof(cl_mem), &mOutBuffer);
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

    void OpenCLRenderContext::dispose() {
        clReleaseMemObject(mOutBuffer);

        clReleaseProgram(mProgram);
        clReleaseKernel(mRenderKernel);
        clReleaseCommandQueue(mCommands);
        clReleaseContext(mContext);
    }
}
