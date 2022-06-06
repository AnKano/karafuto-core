#pragma once

#include "../IRenderContext.hpp"

namespace KCore::OneToOne {
    /**
     * Instead rendering produce image that relate to tile
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

