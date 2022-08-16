#pragma once

#include "glm/glm.hpp"
#include "BaseMesh.hpp"

#include "../../elevation/Elevation.hpp"
#include <vector>

namespace KCore {
    class GridMesh : public BaseMesh {
    public:
        GridMesh(const glm::vec2 &dims, const glm::ivec2 &segments, const glm::bvec2 &flipUVs,
                 const Elevation &elevation);

        ~GridMesh() = default;

    private:
        void createGeneralSurface
                (const glm::vec2 &dims, const glm::ivec2 &segments, const glm::bvec2 &flipUVs,
                 const Elevation &elevation);

        void makeHorizontalBorder
                (const glm::vec2 &dims, const glm::ivec2 &segments, const glm::bvec2 &flipUVs,
                 const float &constraint, const Elevation &elevation, const bool &downBorder);

        void makeVerticalBorder
                (const glm::vec2 &dims, const glm::ivec2 &segments, const glm::bvec2 &flipUVs,
                 const float &constraint, const Elevation &elevation, const bool &rightBorder);
    };
}
