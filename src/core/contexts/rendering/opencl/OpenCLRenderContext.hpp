//
// Created by Anton Shubin on 3/15/22.
//

#ifndef KARAFUTO_CORE_OPENCLRENDERCONTEXT_HPP
#define KARAFUTO_CORE_OPENCLRENDERCONTEXT_HPP

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
        std::size_t mOutImageHeight = 2048;
        std::size_t mOutImageWidth = 2048;
        std::size_t mOutImagePixelBytes = 2;
        std::size_t mOutImageBytes = mOutImageHeight * mOutImageWidth * mOutImagePixelBytes;

    public:
        OpenCLRenderContext(TerrainedWorld *world) : IRenderContext(world) {}

    protected:
        void performLoopStep() override;

    private:
        void initialize() override;

        void dispose() override;

        void childTransform(const std::string &rootQuadcode, const std::string &childQuadcode,
                            unsigned int &offsetX, unsigned int &offsetY, unsigned int &depth);

        void setupRenderKernel();

        void performRenderKernel(const Tile& tile);

        void setupWipeKernel();

        void performWipeKernel();
    };
}


#endif //KARAFUTO_CORE_OPENCLRENDERCONTEXT_HPP
