#pragma once

#include "../IRenderContext.hpp"

namespace KCore::Debug {
    /**
     * Always produce debug render with one image 64 x 64 RGB888 for testing purposes
     */
    class [[maybe_unused]] DebugContext : public IRenderContext {
    public:
        explicit DebugContext(Layer *world) : IRenderContext(world) {}

    protected:
        void initialize() override;

        void performLoopStep() override;

        void dispose() override;
    };
}

