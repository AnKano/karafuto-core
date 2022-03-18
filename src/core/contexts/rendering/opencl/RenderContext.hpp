//
// Created by Anton Shubin on 3/15/22.
//

#ifndef KARAFUTO_CORE_RENDERCONTEXT_HPP
#define KARAFUTO_CORE_RENDERCONTEXT_HPP

#include "../IRenderContext.hpp"

#ifdef __APPLE__

#include <OpenCL/opencl.h>

#endif

#include "Tile.hpp"
#include "misc/BuiltInKernels.inl"

namespace KCore::OpenCL {
    class RenderContext : public IRenderContext {
    private:
        cl_context mContext;
        cl_program mProgram;
        cl_command_queue mCommands;

        cl_kernel mRenderKernel;
        cl_kernel mWipeKernel;

        bool renderKernelSetup = false;
        bool wipeKernelSetup = false;

        cl_mem mOutBuffer[2];
        std::size_t mOutImageHeight = 2048;
        std::size_t mOutImageWidth = 2048;
        std::size_t mOutImagePixelBytes = 2;
        std::size_t mOutImageBytes = mOutImageHeight * mOutImageWidth * mOutImagePixelBytes;

    public:
        RenderContext(BaseWorld *world) : IRenderContext(world) {}

    private:
        void runRenderLoop() override;

        void initialize() override;

        void dispose();

        void childTransform(const std::string &rootQuadcode, const std::string &childQuadcode,
                            unsigned int &offsetX, unsigned int &offsetY, unsigned int &depth);

        void setupRenderKernel();

        void performRenderKernel(const Tile& tile, unsigned int bufferIdx);

        void setupWipeKernel();

        void performWipeKernel(unsigned int bufferIdx);
    };
}


#endif //KARAFUTO_CORE_RENDERCONTEXT_HPP
