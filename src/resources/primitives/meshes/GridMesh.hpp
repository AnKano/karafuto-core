#pragma once

#include "glm/glm.hpp"
#include "BaseMesh.hpp"

#include <vector>

namespace KCore {
    class GridMesh : public BaseMesh {
    public:
        GridMesh(float width, float length,
                 int segmentsX, int segmentsY,
                 bool flipUVsX, bool flipUVsY,
                 const float *heights);

        ~GridMesh() = default;

    private:
        void createMesh() override;

        void createGeneralSurface(float width, float length,
                                  int segmentsX, int segmentsY,
                                  bool flipUVsX, bool flipUVsY,
                                  const std::vector<float> &heights);

        void makeHorizontalBorder(float width, int segments, float constraint,
                                  const std::vector<float> &heights, bool downBorder,
                                  bool flipUVsX, bool flipUVsY);

        void makeVerticalBorder(float height, int segments, float constraint,
                                const std::vector<float> &heights, bool rightBorder,
                                bool flipUVsX, bool flipUVsY);
    };
}
