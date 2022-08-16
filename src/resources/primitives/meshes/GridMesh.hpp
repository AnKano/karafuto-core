#pragma once

#include "glm/glm.hpp"
#include "BaseMesh.hpp"

#include "../../elevation/Elevation.hpp"

#include <vector>

namespace KCore {
    class GridMesh : public BaseMesh {
    private:
        Elevation mElevation;

        glm::vec2 mDims;
        glm::ivec2 mSegments;
        glm::bvec2 mFlipUVs;


    public:
        GridMesh(const glm::vec2 &dims, const glm::ivec2 &segments,
                 const glm::bvec2 &flipUVs, Elevation elevation);

        ~GridMesh() = default;

    private:
        void buildUpSurface();

        void makeHorizontalBorder
                (const float &constraint, const bool &downBorder);

        void makeVerticalBorder
                (const float &constraint, const bool &rightBorder);
    };
}
