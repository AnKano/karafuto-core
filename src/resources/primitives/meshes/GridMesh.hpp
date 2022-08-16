#pragma once

#include "glm/glm.hpp"
#include "BaseMesh.hpp"

#include <vector>

namespace KCore {
    class GridMesh : public BaseMesh {
    public:
        GridMesh(const glm::vec2 &dims, const glm::ivec2 &segments, const glm::bvec2 &flipUVs,
                 const std::vector<std::vector<float>> &heights);

        ~GridMesh() = default;

    private:
        void createGeneralSurface
                (const glm::vec2 &dims, const glm::ivec2 &segments, const glm::bvec2 &flipUVs,
                 const std::vector<std::vector<float>> &heights);

        void makeHorizontalBorder
                (const glm::vec2 &dims, const glm::ivec2 &segments, const glm::bvec2 &flipUVs,
                 const float &constraint, const std::vector<float> &heights, const bool &downBorder);

        void makeVerticalBorder
                (const glm::vec2 &dims, const glm::ivec2 &segments, const glm::bvec2 &flipUVs,
                 const float &constraint, const std::vector<float> &heights, const bool &rightBorder);
    };
}
