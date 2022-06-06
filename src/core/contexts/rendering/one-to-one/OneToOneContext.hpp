#pragma once

#include "../IRenderContext.hpp"

namespace KCore::OneToOne {
    /**
     * Always produce debug render with one image 64 x 64 RGB888 for testing purposes
     */
    class OneToOneContext : public IRenderContext {
    public:
        explicit OneToOneContext(World *world) : IRenderContext(world) {}

    protected:
        void initialize() override;

        void performLoopStep() override;

        void dispose() override;
    };
}

