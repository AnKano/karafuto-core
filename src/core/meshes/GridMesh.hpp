#pragma once

#include "glm/glm.hpp"
#include "BaseMesh.hpp"

#include <vector>

namespace KCore {
    class GridMesh : public BaseMesh {
    public:
        GridMesh(float width, float length, int segmentsX, int segmentsY);

        GridMesh(float width, float length, int segments);

        ~GridMesh() = default;

        void applyHeights(uint8_t *heights, const int &segmentsX, const int &segmentsY);

    private:
        void createMesh() override;

        void createMesh(float width, float length, int segmentsX, int segmentsY);

        void createGeneralSurface(float width, float length, int segmentsX, int segmentsY);
    };
}
