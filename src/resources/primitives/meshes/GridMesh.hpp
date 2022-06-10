#pragma once

#include "glm/glm.hpp"
#include "BaseMesh.hpp"

#include <vector>

namespace KCore {
    class GridMesh : public BaseMesh {
    public:
        GridMesh(float width, float length, int segmentsX, int segmentsY);

        GridMesh(float width, float length, int segments);

        GridMesh(float width, float length, int segmentsX, int segmentsY, const float *heights);

        ~GridMesh() = default;

        void applyHeights(float *heights, const int &segmentsX, const int &segmentsY);

    private:
        void createMesh() override;

        void createMesh(float width, float length, int segmentsX, int segmentsY);

        void createGeneralSurface(float width, float length, int segmentsX, int segmentsY);

        void createGeneralSurface(float width, float length, int segmentsX, int segmentsY,
                                  const std::vector<float> &heights);

        void makeHorizontalBorder(float width, int segments, float constraint,
                                  const std::vector<float> &heights, bool downBorder);

        void makeVerticalBorder(float height, int segments, float constraint,
                                const std::vector<float> &heights, bool rightBorder);
    };
}
