//
// Created by ash on 01.10.2021.
//

#include <string>
#include "GridMesh.hpp"

struct ElevationObject {
    std::vector<uint16_t> kernel;

    std::vector<uint16_t> north;
    std::vector<uint16_t> south;
    std::vector<uint16_t> west;
    std::vector<uint16_t> east;
};

namespace KCore {
    GridMesh::GridMesh(float width, float length, int segments) {
        createMesh(width, length, segments, segments);
    }

    GridMesh::GridMesh(float width, float length, int segmentsX, int segmentsY) : BaseMesh() {
        createMesh(width, length, segmentsX, segmentsY);
    }

    GridMesh::GridMesh(float width, float length, int segmentsX, int segmentsY, const uint8_t *heights) : BaseMesh() {
        auto converted = reinterpret_cast<const uint16_t *>(heights);

        ElevationObject elevation;
        elevation.kernel.resize((segmentsX + 1) * (segmentsY + 1));

        for (int j = 0; j < segmentsY + 1; j++) {
            for (int i = 0; i < segmentsX + 1; i++) {
                const uint16_t height = converted[j * (segmentsX + 1) + i];

                elevation.kernel[j * (segmentsX + 1) + i] = height;

                if (j == 0) elevation.south.push_back(height);
                if (j == segmentsY) elevation.north.push_back(height);

                if (i == 0) elevation.west.push_back(height);
                if (i == segmentsX) elevation.east.push_back(height);
            }
        }

        createGeneralSurface(width, length, segmentsX, segmentsY, elevation.kernel);

        makeHorizontalBorder(width, segmentsX - 1, width / 2, elevation.north, false);
        makeHorizontalBorder(width, segmentsX - 1, -width / 2, elevation.south, true);

        makeVerticalBorder(length, segmentsY - 1, length / 2, elevation.east, false);
        makeVerticalBorder(length, segmentsY - 1, -length / 2, elevation.west, true);
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

    void GridMesh::createGeneralSurface(float width, float length,
                                        int segmentsX, int segmentsY,
                                        const std::vector<uint16_t> &heights) {
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
                mUVs.emplace_back(uvX, 1.0 - uvY);
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
                                        const std::vector<uint16_t> &heights, bool downBorder) {
        const auto vtxCount = mVertices.size();

        const float widthHalf = width / 2.0;
        const float segmentWidth = width / (segments + 1);

        const float uvs = 1.0 / (segments + 1);
        for (int ix = 0; ix <= segments + 1; ix++) {
            const float x = ix * segmentWidth - widthHalf;
            const float height = (heights[ix] <= 5000.0) ? heights[ix] : 0.0;

            mVertices.emplace_back(-x, height, -constraint);

            if (downBorder) mNormals.emplace_back(0.0f, 0.0f, -1.0f);
            if (!downBorder) mNormals.emplace_back(0.0f, 0.0f, 1.0f);

            if (downBorder) mUVs.emplace_back(uvs * ix, 0.0);
            if (!downBorder) mUVs.emplace_back(uvs * ix, 1.0);
        }

        for (int ix = 0; ix <= segments + 1; ix++) {
            const float x = ix * segmentWidth - widthHalf;

            mVertices.emplace_back(-x, -100.0f, -constraint);

            if (downBorder) mNormals.emplace_back(0.0f, 0.0f, -1.0f);
            if (!downBorder) mNormals.emplace_back(0.0f, 0.0f, 1.0f);

            if (downBorder) mUVs.emplace_back(uvs * ix, -0.1);
            if (!downBorder) mUVs.emplace_back(uvs * ix, 1.1);
        }

        for (int ix = 0; ix <= segments; ix++) {
            unsigned int a = ix;
            unsigned int b = ix + 1;
            unsigned int d = (ix + 1) + (segments + 2);
            unsigned int c = ix + (segments + 2);

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
                                      const std::vector<uint16_t> &heights, bool rightBorder) {
        const auto vtxCount = mVertices.size();

        const float heightHalf = height / 2.0;
        const float segmentHeight = height / (segments + 1);

        const float uvs = 1.0 / (segments + 1);
        for (int ix = 0; ix <= segments + 1; ix++) {
            const float x = ix * segmentHeight - heightHalf;

            const float val = heights[heights.size() - ix - 1];
            const float height = (val <= 5000.0) ? val : 0.0;

            mVertices.emplace_back(-constraint, height, x);

            if (rightBorder) mNormals.emplace_back(-1.0f, 0.0f, 0.0f);
            if (!rightBorder) mNormals.emplace_back(1.0f, 0.0f, 0.0f);

            if (!rightBorder) mUVs.emplace_back(0.0, uvs * ix);
            if (rightBorder) mUVs.emplace_back(1.0, uvs * ix);
        }

        for (int ix = 0; ix <= segments + 1; ix++) {
            const float x = ix * segmentHeight - heightHalf;

            mVertices.emplace_back(-constraint, -100.0f, x);

            if (rightBorder) mNormals.emplace_back(-1.0f, 0.0f, 0.0f);
            if (!rightBorder) mNormals.emplace_back(1.0f, 0.0f, 0.0f);

            if (!rightBorder) mUVs.emplace_back(-0.1, uvs * ix);
            if (rightBorder) mUVs.emplace_back(1.1, uvs * ix);
        }

        for (int ix = 0; ix <= segments; ix++) {
            unsigned int a = ix;
            unsigned int b = ix + 1;
            unsigned int d = (ix + 1) + (segments + 2);
            unsigned int c = ix + (segments + 2);

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