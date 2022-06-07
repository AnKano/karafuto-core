#pragma once

#include "../IRenderContext.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "VulkanCore.hpp"

namespace KCore::Vulkan {
    class VulkanRenderContext : public IRenderContext {
    private:
        VulkanCore core;

    public:
        VulkanRenderContext(World *world);

    private:
        void prepareTransform(const std::string &rootQuadcode,
                              const std::string &childQuadcode,
                              glm::mat4 &scaleMatrix,
                              glm::mat4 &translationMatrix);

    protected:
        void initialize() override;

        void performLoopStep() override;

        void dispose() override;
    };
}