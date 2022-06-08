#pragma once

#include "../IRenderContext.hpp"

#if defined(__APPLE__)
#include <OpenCL/opencl.h>
#elif defined(__linux__)
#include <CL/cl.h>
#elif defined(WIN32) || defined(WINDOWS)
#include <CL/cl.h>
#endif

#include "Tile.hpp"
#include "misc/BuiltInKernels.inl"

namespace KCore::OpenCL {
    class OpenCLRenderContext : public IRenderContext {
    private:
        cl_context mContext;
        cl_program mProgram;
        cl_command_queue mCommands;

        cl_kernel mRenderKernel;
        cl_kernel mWipeKernel;

        bool renderKernelSetup = false;
        bool wipeKernelSetup = false;

        cl_mem mOutBuffer;
        int mOutImageHeight = 1024;
        int mOutImageWidth = 1024;
        int mOutImagePixelBytes = 2;
        std::size_t mOutImageBytes = mOutImageHeight * mOutImageWidth * mOutImagePixelBytes;

    public:
        explicit OpenCLRenderContext(Layer *layer) : IRenderContext(layer) {}

    protected:
        void performLoopStep() override;

    private:
        void initialize() override;

        void dispose() override;

        void childTransform(const std::string &rootQuadcode, const std::string &childQuadcode,
                            unsigned int &offsetX, unsigned int &offsetY, unsigned int &depth);

        void setupRenderKernel();

        void performRenderKernel(const Tile &tile);

        void setupWipeKernel();

        void performWipeKernel();
    };
}
