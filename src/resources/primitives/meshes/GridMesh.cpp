//
// Created by ash on 01.10.2021.
//

#include <string>
#include "GridMesh.hpp"

struct ElevationObject {
    std::vector<float> kernel;

    std::vector<float> north;
    std::vector<float> south;
    std::vector<float> west;
    std::vector<float> east;
};

namespace KCore {
    GridMesh::GridMesh(float width, float length,
                       int segmentsX, int segmentsY,
                       bool flipUVsX, bool flipUVsY,
                       const float *heights) : BaseMesh() {
        ElevationObject elevation;
        elevation.kernel.resize((segmentsX + 1) * (segmentsY + 1));

        for (int j = 0; j < segmentsY + 1; j++) {
            for (int i = 0; i < segmentsX + 1; i++) {
                const float height = heights[j * (segmentsX + 1) + i];

                elevation.kernel[j * (segmentsX + 1) + i] = height;

                if (j == 0) elevation.south.push_back(height);
                if (j == segmentsY) elevation.north.push_back(height);

                if (i == 0) elevation.west.push_back(height);
                if (i == segmentsX) elevation.east.push_back(height);
            }
        }

        createGeneralSurface(width, length, segmentsX, segmentsY, flipUVsX, flipUVsY, elevation.kernel);

        makeHorizontalBorder(width, segmentsX, width / 2, elevation.north, false, flipUVsX, flipUVsY);
        makeHorizontalBorder(width, segmentsX, -width / 2, elevation.south, true, flipUVsX, flipUVsY);

        makeVerticalBorder(length, segmentsY, length / 2, elevation.east, false, flipUVsX, flipUVsY);
        makeVerticalBorder(length, segmentsY, -length / 2, elevation.west, true, flipUVsX, flipUVsY);
    }

    void GridMesh::createMesh() { }

    void GridMesh::createGeneralSurface(float width, float length,
                                        int segmentsX, int segmentsY,
                                        bool flipUVsX, bool flipUVsY,
                                        const std::vector<float> &heights) {
        const float widthHalf = width / 2.0f;
        const float heightHalf = length / 2.0f;

        const int gridX1 = segmentsX + 1;
        const int gridY1 = segmentsY + 1;

        const float segment_width = width / segmentsX;
        const float segment_height = length / segmentsY;

        for (int iy = 0; iy < gridY1; iy++) {
            const float y = iy * segment_height - heightHalf;
            for (int ix = 0; ix < gridX1; ix++) {
                const float x = ix * segment_width - widthHalf;

                const float val = heights[iy * (gridX1) + ix];
                const auto height = (val <= 5000.0f) ? heights[iy * (gridX1) + ix] : 0.0;

                mVertices.emplace_back(-1 * x, height, -1 * y);
                mNormals.emplace_back(0.0f, 1.0f, 0.0f);

                float uvX = (float) ix / ((float) segmentsX);
                float uvY = (float) iy / ((float) segmentsY);

                float modUvX = (flipUVsX) ? 1.0f - uvX : uvX;
                float modUvY = (flipUVsY) ? 1.0f - uvY : uvY;

                mUVs.emplace_back(modUvX, modUvY);
            }
        }

        for (int iy = 0; iy < segmentsY; iy++) {
            for (int ix = 0; ix < segmentsX; ix++) {
                const unsigned int a = ix + gridX1 * iy;
                const unsigned int b = ix + gridX1 * (iy + 1);
                const unsigned int c = (ix + 1) + gridX1 * (iy + 1);
                const unsigned int d = (ix + 1) + gridX1 * iy;

                mIndices.insert(mIndices.end(), {a, b, c});
                mIndices.insert(mIndices.end(), {a, c, d});
            }
        }
    }

    void GridMesh::makeHorizontalBorder(float width, int segments, float constraint,
                                        const std::vector<float> &heights, bool downBorder,
                                        bool flipUVsX, bool flipUVsY) {
        const auto vtxCount = mVertices.size();

        const float widthHalf = width / 2.0;
        const float segmentWidth = width / segments;

        const int grid1 = segments + 1;

        const float uvs = 1.0f / segments;
        for (int ix = 0; ix < grid1; ix++) {
            const float x = ix * segmentWidth - widthHalf;
            const float height = (heights[ix] <= 5000.0f) ? heights[ix] : 0.0f;

            mVertices.emplace_back(-x, height, -constraint);

            mNormals.emplace_back(0.0f, 1.0f, 0.0f);

            float modUV = (flipUVsX) ? 1.0f - (uvs * ix) : uvs * ix;

            if (downBorder) mUVs.emplace_back(modUV, (flipUVsY) ? 0.0f : 1.0f);
            if (!downBorder) mUVs.emplace_back(modUV, (flipUVsY) ? 1.0f : 0.0f);
        }

        for (int ix = 0; ix < grid1; ix++) {
            const float x = ix * segmentWidth - widthHalf;

            mVertices.emplace_back(-x, -100.0f, -constraint);

            mNormals.emplace_back(0.0f, 1.0f, 0.0f);

            float modUV = (flipUVsX) ? 1.0f - (uvs * ix) : uvs * ix;

            if (downBorder) mUVs.emplace_back(modUV, (flipUVsY) ? -0.1f : 1.1f);
            if (!downBorder) mUVs.emplace_back(modUV, (flipUVsY) ? 1.1f : -0.1f);
        }

        for (int ix = 0; ix < segments; ix++) {
            unsigned int a = ix;
            unsigned int b = ix + 1;
            unsigned int d = (ix + 1) + (segments + 1);
            unsigned int c = ix + (segments + 1);

            a += vtxCount;
            b += vtxCount;
            c += vtxCount;
            d += vtxCount;

            if (downBorder) {
                mIndices.insert(mIndices.end(), {a, b, d});
                mIndices.insert(mIndices.end(), {a, d, c});
            } else {
                mIndices.insert(mIndices.end(), {d, b, a});
                mIndices.insert(mIndices.end(), {c, d, a});
            }
        }
    }

    void GridMesh::makeVerticalBorder(float height, int segments, float constraint,
                                      const std::vector<float> &heights, bool rightBorder,
                                      bool flipUVsX, bool flipUVsY) {
        const auto vtxCount = mVertices.size();

        const float heightHalf = height / 2.0f;
        const float segmentHeight = height / segments;

        const int grid1 = segments + 1;

        const float uvs = 1.0f / (segments + 1);
        for (int ix = 0; ix < grid1; ix++) {
            const float x = ix * segmentHeight - heightHalf;

            const float val = heights[heights.size() - ix - 1];
            const float height = (val <= 5000.0f) ? val : 0.0f;

            mVertices.emplace_back(-constraint, height, x);

            mNormals.emplace_back(0.0f, 1.0f, 0.0f);

            float modUV = (!flipUVsY) ? 1.0f - (uvs * ix) : uvs * ix;

            if (rightBorder) mUVs.emplace_back((flipUVsX) ? 0.0f : 1.0f, modUV);
            if (!rightBorder) mUVs.emplace_back((flipUVsX) ? 1.0f : 0.0f, modUV);
        }

        for (int ix = 0; ix < grid1; ix++) {
            const float x = ix * segmentHeight - heightHalf;

            mVertices.emplace_back(-constraint, -100.0f, x);

            mNormals.emplace_back(0.0f, 1.0f, 0.0f);

            float modUV = (!flipUVsY) ? 1.0f - (uvs * ix) : uvs * ix;

            if (rightBorder) mUVs.emplace_back((flipUVsX) ? -0.1f : 1.1f, modUV);
            if (!rightBorder) mUVs.emplace_back((flipUVsX) ? 1.1f : -0.1f, modUV);
        }

        for (int ix = 0; ix < segments; ix++) {
            unsigned int a = ix;
            unsigned int b = ix + 1;
            unsigned int d = (ix + 1) + (segments + 1);
            unsigned int c = ix + (segments + 1);

            a += vtxCount;
            b += vtxCount;
            c += vtxCount;
            d += vtxCount;

            if (rightBorder) {
                mIndices.insert(mIndices.end(), {a, b, d});
                mIndices.insert(mIndices.end(), {a, d, c});
            } else {
                mIndices.insert(mIndices.end(), {d, b, a});
                mIndices.insert(mIndices.end(), {c, d, a});
            }
        }
    }
}