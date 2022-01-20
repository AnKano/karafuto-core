#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/intersect.hpp"
#include "glm/gtx/vector_angle.hpp"

#include <array>
#include <iostream>

namespace KCore {
    class FrustumCulling {
    private:
        float nxX{}, nxY{}, nxZ{}, nxW{};
        float pxX{}, pxY{}, pxZ{}, pxW{};
        float nyX{}, nyY{}, nyZ{}, nyW{};
        float pyX{}, pyY{}, pyZ{}, pyW{};
        float nzX{}, nzY{}, nzZ{}, nzW{};
        float pzX{}, pzY{}, pzZ{}, pzW{};

        glm::mat4 projectViewMatrix{};
    public:
        FrustumCulling() = default;

        void update_frustum(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix) {
            this->projectViewMatrix = projectionMatrix * viewMatrix;

            nxX = projectViewMatrix[0][3] + projectViewMatrix[0][0];
            nxY = projectViewMatrix[1][3] + projectViewMatrix[1][0];
            nxZ = projectViewMatrix[2][3] + projectViewMatrix[2][0];
            nxW = projectViewMatrix[3][3] + projectViewMatrix[3][0];

            pxX = projectViewMatrix[0][3] - projectViewMatrix[0][0];
            pxY = projectViewMatrix[1][3] - projectViewMatrix[1][0];
            pxZ = projectViewMatrix[2][3] - projectViewMatrix[2][0];
            pxW = projectViewMatrix[3][3] - projectViewMatrix[3][0];

            nyX = projectViewMatrix[0][3] + projectViewMatrix[0][1];
            nyY = projectViewMatrix[1][3] + projectViewMatrix[1][1];
            nyZ = projectViewMatrix[2][3] + projectViewMatrix[2][1];
            nyW = projectViewMatrix[3][3] + projectViewMatrix[3][1];

            pyX = projectViewMatrix[0][3] - projectViewMatrix[0][1];
            pyY = projectViewMatrix[1][3] - projectViewMatrix[1][1];
            pyZ = projectViewMatrix[2][3] - projectViewMatrix[2][1];
            pyW = projectViewMatrix[3][3] - projectViewMatrix[3][1];

            nzX = projectViewMatrix[0][3] + projectViewMatrix[0][2];
            nzY = projectViewMatrix[1][3] + projectViewMatrix[1][2];
            nzZ = projectViewMatrix[2][3] + projectViewMatrix[2][2];
            nzW = projectViewMatrix[3][3] + projectViewMatrix[3][2];

            pzX = projectViewMatrix[0][3] - projectViewMatrix[0][2];
            pzY = projectViewMatrix[1][3] - projectViewMatrix[1][2];
            pzZ = projectViewMatrix[2][3] - projectViewMatrix[2][2];
            pzW = projectViewMatrix[3][3] - projectViewMatrix[3][2];
        }

        [[nodiscard]]
        bool test_box(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) const {
            return nxX * (nxX < 0 ? minX : maxX) +
                   nxY * (nxY < 0 ? minY : maxY) +
                   nxZ * (nxZ < 0 ? minZ : maxZ) >= -nxW &&
                   pxX * (pxX < 0 ? minX : maxX) +
                   pxY * (pxY < 0 ? minY : maxY) +
                   pxZ * (pxZ < 0 ? minZ : maxZ) >= -pxW &&
                   nyX * (nyX < 0 ? minX : maxX) +
                   nyY * (nyY < 0 ? minY : maxY) +
                   nyZ * (nyZ < 0 ? minZ : maxZ) >= -nyW &&
                   pyX * (pyX < 0 ? minX : maxX) +
                   pyY * (pyY < 0 ? minY : maxY) +
                   pyZ * (pyZ < 0 ? minZ : maxZ) >= -pyW &&
                   nzX * (nzX < 0 ? minX : maxX) +
                   nzY * (nzY < 0 ? minY : maxY) +
                   nzZ * (nzZ < 0 ? minZ : maxZ) >= -nzW &&
                   pzX * (pzX < 0 ? minX : maxX) +
                   pzY * (pzY < 0 ? minY : maxY) +
                   pzZ * (pzZ < 0 ? minZ : maxZ) >= -pzW;
        }
    };
}
