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

    void GridMesh::createMesh() {
        createMesh(1.0f, 1.0f, 1, 1);
    }

    void GridMesh::createMesh(float width, float length, int segmentsX, int segmentsY) {
        createGeneralSurface(width, length, segmentsX, segmentsY);

        // North and South borders
        createBorderSurfaceX(width, segmentsX, -0.5f, 0.0f, -0.1f);
        createBorderSurfaceX(width, segmentsX, 0.5f, 1.0f, 1.1f);

        // East and West borders
        createBorderSurfaceY(length, segmentsY, -0.5f, 0.0f, -0.1f);
        createBorderSurfaceY(length, segmentsY, 0.5f, 1.0f, 1.1f);
    }

    void GridMesh::createGeneralSurface(float width, float length, int segmentsX, int segmentsY) {
        segmentsY *= 2;
        segmentsX *= 2;

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

                mVertices.emplace_back(x, 0.0f, y);
                mNormals.emplace_back(0.0f, 1.0f, 0.0f);

                float uvX = (float) ix / ((float) segmentsX);
                float uvY = (float) iy / ((float) segmentsY);

                mUVs.emplace_back(uvX, 1.0f - uvY);
            }
        }

        for (int iy = 0; iy < segmentsY; iy += 2) {
            for (int ix = 0; ix < segmentsX; ix += 2) {
                const uint32_t a = ix + gridX * iy;
                const uint32_t b = ix + gridX * (iy + 2);

                // index of mCenter of each segment
                const uint32_t x = (ix + 1) + gridX * (iy + 1);

                const uint32_t c = (ix + 2) + gridX * (iy + 2);
                const uint32_t d = (ix + 2) + gridX * iy;

                mIndices.insert(mIndices.end(), {a, b, x});
                mIndices.insert(mIndices.end(), {b, c, x});
                mIndices.insert(mIndices.end(), {c, d, x});
                mIndices.insert(mIndices.end(), {d, a, x});
            }
        }
    }

    void GridMesh::createBorderSurfaceX(float width, int segmentsX, float constraint,
                                        float uvConstant, float uvInterpolatedConstant) {
        uint32_t verticesCount = mVertices.size();

        int segmentsY = 1;

        const float widthHalf = width / 2.0f;
        const int gridX = segmentsX + 1;
        const float segmentWidth = width / ((float) segmentsX);

        for (int iy = 0; iy < 2; iy++) {
            for (int ix = 0; ix < gridX; ix++) {
                float x = ((float) ix * segmentWidth) - widthHalf;
                float z = (iy == 0) ? 0.0f : -1.0f;

                float uvX = (float) ix / ((float) segmentsX);
                float uvY = (iy == 0) ? uvConstant : uvInterpolatedConstant;

                mVertices.emplace_back(x, z, constraint);
                mNormals.emplace_back(0.0f, 1.0f, 0.0f);
                mUVs.emplace_back(uvX, 1.0f - uvY);
            }
        }

        for (int iy = 0; iy < segmentsY; iy++) {
            for (int ix = 0; ix < segmentsX; ix++) {
                uint32_t a = ix + gridX * iy;
                uint32_t b = ix + gridX * (iy + 1);
                uint32_t c = (ix + 1) + gridX * (iy + 1);
                uint32_t d = (ix + 1) + gridX * iy;

                a += verticesCount;
                b += verticesCount;
                c += verticesCount;
                d += verticesCount;

                mIndices.insert(mIndices.end(), {a, b, d});
                mIndices.insert(mIndices.end(), {b, c, d});
            }
        }
    }

    void GridMesh::createBorderSurfaceY(float length, int segmentsY, float constraint,
                                        float uvConstant, float uvInterpolatedConstant) {
        uint32_t verticesCount = mVertices.size();

        int segmentsX = 1;

        const float heightHalf = length / 2.0f;
        const int gridY = segmentsY + 1;
        const float segmentHeight = length / ((float) segmentsY);

        for (int iy = 0; iy < gridY; iy++) {
            for (int ix = 0; ix < 2; ix++) {
                float y = ((float) iy * segmentHeight) - heightHalf;
                float z = (ix == 0) ? 0.0f : -1.0f;

                float uvX = (iy == 0) ? uvConstant : uvInterpolatedConstant;
                float uvY = (float) iy / ((float) segmentsY);

                mVertices.emplace_back(constraint, z, y);
                mNormals.emplace_back(0.0f, 1.0f, 0.0f);
                mUVs.emplace_back(uvX, 1.0f - uvY);
            }
        }

        for (int iy = 0; iy < segmentsY; iy++) {
            for (int ix = 0; ix < segmentsX; ix++) {
                uint32_t a = ix + 2 * iy;
                uint32_t b = ix + 2 * (iy + 1);
                uint32_t c = (ix + 1) + 2 * (iy + 1);
                uint32_t d = (ix + 1) + 2 * iy;

                a += verticesCount;
                b += verticesCount;
                c += verticesCount;
                d += verticesCount;

                mIndices.insert(mIndices.end(), {a, b, d});
                mIndices.insert(mIndices.end(), {b, c, d});
            }
        }
    }
}