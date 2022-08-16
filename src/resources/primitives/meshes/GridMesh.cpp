#include "GridMesh.hpp"

struct ElevationObject {
    std::vector<std::vector<float>> kernel;
    std::vector<float> north, south, west, east;
};

namespace KCore {
    GridMesh::GridMesh
            (const glm::vec2 &dims, const glm::ivec2 &segments, const glm::bvec2 &flipUVs,
             const std::vector<std::vector<float>> &heights) : BaseMesh() {
        ElevationObject elevation;
        elevation.kernel = heights;

        for (int j = 0; j < segments.y + 1; j++) {
            for (int i = 0; i < segments.x + 1; i++) {
                const float height = heights[j][i];

                if (j == 0) elevation.south.push_back(height);
                if (j == segments.y) elevation.north.push_back(height);

                if (i == 0) elevation.west.push_back(height);
                if (i == segments.x) elevation.east.push_back(height);
            }
        }

        createGeneralSurface(dims, segments, flipUVs, elevation.kernel);

        makeHorizontalBorder(dims, segments, flipUVs, dims.x / 2, elevation.north, false);
        makeHorizontalBorder(dims, segments, flipUVs, -dims.x / 2, elevation.south, true);

        makeVerticalBorder(dims, segments, flipUVs, dims.y / 2, elevation.east, false);
        makeVerticalBorder(dims, segments, flipUVs, -dims.y / 2, elevation.west, true);
    }

    void GridMesh::createGeneralSurface
            (const glm::vec2 &dims, const glm::ivec2 &segments, const glm::bvec2 &flipUVs,
             const std::vector<std::vector<float>> &heights) {
        const float length_x = dims.x;
        const float length_y = dims.y;

        const float length_x_half = length_x / 2.0f;
        const float length_y_half = length_y / 2.0f;

        const int grid_x = segments.x + 1;
        const int grid_y = segments.y + 1;

        const float segment_x_length = length_x / (float) segments.x;
        const float segment_y_length = length_y / (float) segments.y;

        for (int iy = 0; iy < grid_y; iy++) {
            const float y = (float) iy * segment_y_length - length_y_half;
            for (int ix = 0; ix < grid_x; ix++) {
                const float x = (float) ix * segment_x_length - length_x_half;

                const float val = heights[iy][ix];
                const auto height = (val <= 5000.0f) ? val * 0.001 : 0.0;

                mVertices.emplace_back(-1 * x, height, -1 * y);
                mNormals.emplace_back(0.0f, 1.0f, 0.0f);

                float uvX = (float) ix / ((float) segments.x);
                float uvY = (float) iy / ((float) segments.y);

                float modUvX = (flipUVs.x) ? 1.0f - uvX : uvX;
                float modUvY = (flipUVs.y) ? 1.0f - uvY : uvY;

                mUVs.emplace_back(modUvX, modUvY);
            }
        }

        for (int iy = 0; iy < segments.y; iy++) {
            for (int ix = 0; ix < segments.x; ix++) {
                const unsigned int a = ix + grid_x * iy;
                const unsigned int b = ix + grid_x * (iy + 1);
                const unsigned int c = (ix + 1) + grid_x * (iy + 1);
                const unsigned int d = (ix + 1) + grid_x * iy;

                mIndices.insert(mIndices.end(), {a, b, c});
                mIndices.insert(mIndices.end(), {a, c, d});
            }
        }
    }

    void GridMesh::makeHorizontalBorder
            (const glm::vec2 &dims, const glm::ivec2 &segments, const glm::bvec2 &flipUVs,
             const float &constraint, const std::vector<float> &heights, const bool &downBorder) {
        const auto length_x = dims.x;
        const int grid_x = segments.x + 1;
        const float length_x_half = length_x / 2.0f;
        const float segment_x_length = length_x / (float) segments.x;

        const auto vtxCountCopy = mVertices.size();

        const float uvs = 1.0f / (float) segments.x;
        for (int ix = 0; ix < grid_x; ix++) {
            const float x = (float) ix * segment_x_length - length_x_half;
            const float height = (heights[ix] <= 5000.0f) ? heights[ix] * 0.001f : 0.0f;

            mVertices.emplace_back(-x, height, -constraint);

            mNormals.emplace_back(0.0f, 1.0f, 0.0f);

            float modUV = (flipUVs.x) ? 1.0f - (uvs * (float) ix) : uvs * (float) ix;

            if (downBorder) mUVs.emplace_back(modUV, (flipUVs.y) ? 0.0f : 1.0f);
            if (!downBorder) mUVs.emplace_back(modUV, (flipUVs.y) ? 1.0f : 0.0f);
        }

        for (int ix = 0; ix < grid_x; ix++) {
            const float x = (float) ix * segment_x_length - length_x_half;

            mVertices.emplace_back(-x, -1.0f, -constraint);

            mNormals.emplace_back(0.0f, 1.0f, 0.0f);

            float modUV = (flipUVs.x) ? 1.0f - (uvs * (float) ix) : uvs * (float) ix;

            if (downBorder) mUVs.emplace_back(modUV, (flipUVs.y) ? -0.1f : 1.1f);
            if (!downBorder) mUVs.emplace_back(modUV, (flipUVs.y) ? 1.1f : -0.1f);
        }

        for (int ix = 0; ix < segments.x; ix++) {
            unsigned int a = ix;
            unsigned int b = ix + 1;
            unsigned int d = (ix + 1) + (segments.x + 1);
            unsigned int c = ix + (segments.x + 1);

            a += vtxCountCopy;
            b += vtxCountCopy;
            c += vtxCountCopy;
            d += vtxCountCopy;

            if (downBorder) {
                mIndices.insert(mIndices.end(), {a, b, d});
                mIndices.insert(mIndices.end(), {a, d, c});
            } else {
                mIndices.insert(mIndices.end(), {d, b, a});
                mIndices.insert(mIndices.end(), {c, d, a});
            }
        }
    }

    void GridMesh::makeVerticalBorder
            (const glm::vec2 &dims, const glm::ivec2 &segments, const glm::bvec2 &flipUVs,
             const float &constraint, const std::vector<float> &heights, const bool &rightBorder) {
        const auto length_y = dims.y;
        const int grid_y = segments.y + 1;
        const float length_y_half = length_y / 2.0f;
        const float segment_y_length = length_y / (float) segments.y;

        const auto vtxCountCopy = mVertices.size();

        const float uvs = 1.0f / (float) segments.y;
        for (int ix = 0; ix < grid_y; ix++) {
            const float x = (float) ix * segment_y_length - length_y_half;

            const float val = heights[heights.size() - ix - 1];
            const float height = (val <= 5000.0f) ? val * 0.001f : 0.0f;

            mVertices.emplace_back(-constraint, height, x);

            mNormals.emplace_back(0.0f, 1.0f, 0.0f);

            float modUV = (!flipUVs.y) ? 1.0f - (uvs * (float) ix) : uvs * (float) ix;

            if (rightBorder) mUVs.emplace_back((flipUVs.x) ? 0.0f : 1.0f, modUV);
            if (!rightBorder) mUVs.emplace_back((flipUVs.x) ? 1.0f : 0.0f, modUV);
        }

        for (int ix = 0; ix < grid_y; ix++) {
            const float x = (float) ix * segment_y_length - length_y_half;

            mVertices.emplace_back(-constraint, -1.0f, x);

            mNormals.emplace_back(0.0f, 1.0f, 0.0f);

            float modUV = (!flipUVs.y) ? 1.0f - (uvs * (float) ix) : uvs * (float) ix;

            if (rightBorder) mUVs.emplace_back((flipUVs.x) ? -0.1f : 1.1f, modUV);
            if (!rightBorder) mUVs.emplace_back((flipUVs.x) ? 1.1f : -0.1f, modUV);
        }

        for (int ix = 0; ix < segments.y; ix++) {
            unsigned int a = ix;
            unsigned int b = ix + 1;
            unsigned int d = (ix + 1) + (segments.y + 1);
            unsigned int c = ix + (segments.y + 1);

            a += vtxCountCopy;
            b += vtxCountCopy;
            c += vtxCountCopy;
            d += vtxCountCopy;

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