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

        glm::mat4 mProjectViewMatrix{};
    public:
        FrustumCulling() = default;

        void updateFrustum(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix) {
            mProjectViewMatrix = projectionMatrix * viewMatrix;

            nxX = mProjectViewMatrix[0][3] + mProjectViewMatrix[0][0];
            nxY = mProjectViewMatrix[1][3] + mProjectViewMatrix[1][0];
            nxZ = mProjectViewMatrix[2][3] + mProjectViewMatrix[2][0];
            nxW = mProjectViewMatrix[3][3] + mProjectViewMatrix[3][0];

            pxX = mProjectViewMatrix[0][3] - mProjectViewMatrix[0][0];
            pxY = mProjectViewMatrix[1][3] - mProjectViewMatrix[1][0];
            pxZ = mProjectViewMatrix[2][3] - mProjectViewMatrix[2][0];
            pxW = mProjectViewMatrix[3][3] - mProjectViewMatrix[3][0];

            nyX = mProjectViewMatrix[0][3] + mProjectViewMatrix[0][1];
            nyY = mProjectViewMatrix[1][3] + mProjectViewMatrix[1][1];
            nyZ = mProjectViewMatrix[2][3] + mProjectViewMatrix[2][1];
            nyW = mProjectViewMatrix[3][3] + mProjectViewMatrix[3][1];

            pyX = mProjectViewMatrix[0][3] - mProjectViewMatrix[0][1];
            pyY = mProjectViewMatrix[1][3] - mProjectViewMatrix[1][1];
            pyZ = mProjectViewMatrix[2][3] - mProjectViewMatrix[2][1];
            pyW = mProjectViewMatrix[3][3] - mProjectViewMatrix[3][1];

            nzX = mProjectViewMatrix[0][3] + mProjectViewMatrix[0][2];
            nzY = mProjectViewMatrix[1][3] + mProjectViewMatrix[1][2];
            nzZ = mProjectViewMatrix[2][3] + mProjectViewMatrix[2][2];
            nzW = mProjectViewMatrix[3][3] + mProjectViewMatrix[3][2];

            pzX = mProjectViewMatrix[0][3] - mProjectViewMatrix[0][2];
            pzY = mProjectViewMatrix[1][3] - mProjectViewMatrix[1][2];
            pzZ = mProjectViewMatrix[2][3] - mProjectViewMatrix[2][2];
            pzW = mProjectViewMatrix[3][3] - mProjectViewMatrix[3][2];
        }

        [[nodiscard]]
        bool testAABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) const {
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
