#pragma once

#include "glm/glm.hpp"
#include "BaseMesh.hpp"

#include <vector>

namespace KCore {
    class GridMesh : public BaseMesh {
    public:
        GridMesh(float width, float length, int segmentsX, int segmentsY);

        GridMesh(float width, float length, int segments);

        GridMesh(float width, float length, int segmentsX, int segmentsY, const uint8_t *heights);

        ~GridMesh() = default;

        void applyHeights(uint8_t *heights, const int &segmentsX, const int &segmentsY);

    private:
        void createMesh() override;

        void createMesh(float width, float length, int segmentsX, int segmentsY);

        void createGeneralSurface(float width, float length, int segmentsX, int segmentsY);

        void createGeneralSurface(float width, float length, int segmentsX, int segmentsY,
                                  const std::vector<uint16_t> &heights);

        void makeHorizontalBorder(float width, int segments, float constraint,
                                  const std::vector<uint16_t> &heights, bool downBorder);

        void makeVerticalBorder(float height, int segments, float constraint,
                                const std::vector<uint16_t> &heights, bool rightBorder);
    };
}
