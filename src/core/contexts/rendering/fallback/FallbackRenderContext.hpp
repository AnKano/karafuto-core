#pragma once

#include "../IRenderContext.hpp"

namespace KCore::Fallback {
    class FallbackRenderContext : public IRenderContext {
    public:
        FallbackRenderContext(TerrainedWorld *world) : IRenderContext(world) {}

    protected:
        void initialize() override;

        void performLoopStep() override;

        void dispose() override;
    };
}

