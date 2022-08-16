#pragma once

#include <vector>

#include "glm/glm.hpp"

#include "BaseMesh.hpp"
#include "../resources/elevation/Elevation.hpp"

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

        void produceNorthBorder();

        void produceSouthBorder();

        void produceEastBorder();

        void produceWestBorder();

        void connectLastVertices
                (const int &num, const int &segments, const bool &opposite);

        void buildBorder
                (const bool &horizontal, const bool &opposite);
    };
}
