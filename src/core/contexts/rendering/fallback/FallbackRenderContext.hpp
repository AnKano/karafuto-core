#pragma once

#include "../IRenderContext.hpp"

namespace KCore::Fallback {
    class FallbackRenderContext : public IRenderContext {
    public:
        FallbackRenderContext(World *world) : IRenderContext(world) {}

    protected:
        void initialize() override;

        void performLoopStep() override;

        void dispose() override;
    };
}

