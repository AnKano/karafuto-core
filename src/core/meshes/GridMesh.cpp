//
// Created by ash on 01.10.2021.
//

#include <string>
#include "GridMesh.hpp"

namespace KCore {
    GridMesh::GridMesh(float width, float length, int segments) {
        createMesh(width, length, segments, segments);
    }

    GridMesh::GridMesh(float width, float length, int segmentsX, int segmentsY) : BaseMesh() {
        createMesh(width, length, segmentsX, segmentsY);
    }

    void GridMesh::applyHeights(uint8_t *heights, const int &segmentsX, const int &segmentsY) {
        auto converted = reinterpret_cast<uint16_t *>(heights);

        for (int i = 0; i < (segmentsX + 1) * (segmentsY + 1); i++)
            mVertices[i].y = (converted[i] > 7000.0f) ? 0.0f : converted[i];
    }

    void GridMesh::createMesh() {
        createMesh(1.0f, 1.0f, 1, 1);
    }

    void GridMesh::createMesh(float width, float length, int segmentsX, int segmentsY) {
        createGeneralSurface(width, length, segmentsX, segmentsY);
    }

    void GridMesh::createGeneralSurface(float width, float length, int segmentsX, int segmentsY) {
        const float widthHalf = width / 2.0f;
        const float heightHalf = length / 2.0f;

        const int gridX = segmentsX + 1;
        const int gridY = segmentsY + 1;

        const float segmentWidth = width / ((float) segmentsX);
        const float segmentHeight = length / ((float) segmentsY);

        for (int iy = 0; iy < gridY; iy++) {
            for (int ix = 0; ix < gridX; ix++) {
                float x = ((float) ix * segmentWidth) - widthHalf;
                float y = ((float) iy * segmentHeight) - heightHalf;

                mVertices.emplace_back(-1 * x, 0.0f, -1 * y);
                mNormals.emplace_back(0.0f, 1.0f, 0.0f);

                float uvX = (float) ix / ((float) segmentsX);
                float uvY = (float) iy / ((float) segmentsY);

                mUVs.emplace_back(uvX, 1.0 - uvY);
            }
        }

        for (int iy = 0; iy < segmentsY; iy++) {
            for (int ix = 0; ix < segmentsX; ix++) {
                const uint32_t a = ix + gridX * iy;
                const uint32_t b = ix + gridX * (iy + 1);

                const uint32_t c = (ix + 1) + gridX * (iy + 1);
                const uint32_t d = (ix + 1) + gridX * iy;

                mIndices.insert(mIndices.end(), {a, b, c});
                mIndices.insert(mIndices.end(), {a, c, d});
            }
        }
    }
}