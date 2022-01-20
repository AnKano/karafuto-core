#pragma once

#include "glm/glm.hpp"
#include "IMesh.hpp"

#include <vector>

namespace KCore {
    class GridMesh : public IMesh {
    public:
        GridMesh(float width, float length, int segmentsX, int segmentsY);

        GridMesh(float width, float length, int segments);

        ~GridMesh() = default;

    private:
        void createMesh() override;

        void createMesh(float width, float length, int segmentsX, int segmentsY);

        void createGeneralSurface(float width, float length, int segmentsX, int segmentsY);

        void createBorderSurfaceX(float width, int segmentsX, float constraint,
                                  float uvConstant, float uvInterpolatedConstant);

        void createBorderSurfaceY(float length, int segmentsY, float constraint,
                                  float uvConstant, float uvInterpolatedConstant);
    };
}
