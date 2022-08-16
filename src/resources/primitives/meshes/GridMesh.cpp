#include "GridMesh.hpp"

#include <utility>

namespace KCore {
    GridMesh::GridMesh
            (const glm::vec2 &dims, const glm::ivec2 &segments,
             const glm::bvec2 &flipUVs, Elevation elevation)
            : BaseMesh(),
              mDims(dims), mSegments(segments),
              mFlipUVs(flipUVs), mElevation(std::move(elevation)) {
        buildUpSurface();

        makeHorizontalBorder(dims.x / 2, false);
        makeHorizontalBorder(-dims.x / 2, true);

        makeVerticalBorder(dims.y / 2, false);
        makeVerticalBorder(-dims.y / 2, true);
    }

    void GridMesh::buildUpSurface() {
        const float length_x = mDims.x;
        const float length_y = mDims.y;

        const float length_x_half = length_x / 2.0f;
        const float length_y_half = length_y / 2.0f;

        const int loop_steps_x = mSegments.x + 1;
        const int loop_steps_y = mSegments.y + 1;

        const float segment_x_length = length_x / (float) mSegments.x;
        const float segment_y_length = length_y / (float) mSegments.y;

        for (int iy = 0; iy < loop_steps_y; iy++) {
            for (int ix = 0; ix < loop_steps_x; ix++) {
                const float x = (float) ix * segment_x_length - length_x_half;
                const float y = (float) iy * segment_y_length - length_y_half;

                const float val = mElevation.get(iy, ix);

                const auto height = (val <= 5000.0f) ? val * 0.001f : 0.0f;

                mVertices.emplace_back(x, height, y);
                mNormals.emplace_back(0.0f, 1.0f, 0.0f);

                float uvX = (float) ix / ((float) mSegments.x);
                float uvY = (float) iy / ((float) mSegments.y);

                float modUvX = (mFlipUVs.x) ? 1.0f - uvX : uvX;
                float modUvY = (mFlipUVs.y) ? 1.0f - uvY : uvY;

                mUVs.emplace_back(modUvX, modUvY);
            }
        }

        for (int iy = 0; iy < mSegments.y; iy++) {
            for (int ix = 0; ix < mSegments.x; ix++) {
                const unsigned int a = ix + loop_steps_x * iy;
                const unsigned int b = ix + loop_steps_x * (iy + 1);
                const unsigned int c = (ix + 1) + loop_steps_x * (iy + 1);
                const unsigned int d = (ix + 1) + loop_steps_x * iy;

                mIndices.insert(mIndices.end(), {a, b, c});
                mIndices.insert(mIndices.end(), {a, c, d});
            }
        }
    }

    void GridMesh::makeHorizontalBorder
            (const float &constraint, const bool &downBorder) {
        auto heights = mElevation.getRow((downBorder) ? 0 : mSegments.y);

        const auto length_x = mDims.x;
        const int grid_x = mSegments.x + 1;
        const float length_x_half = length_x / 2.0f;
        const float segment_x_length = length_x / (float) mSegments.x;

        const auto vtxCountCopy = mVertices.size();

        const float uvs = 1.0f / (float) mSegments.x;
        for (int ix = 0; ix < grid_x; ix++) {
            const float x = (float) ix * segment_x_length - length_x_half;
            const float height = (heights[ix] <= 5000.0f) ? heights[ix] * 0.001f : 0.0f;

            mVertices.emplace_back(x, height, constraint);

            mNormals.emplace_back(0.0f, 1.0f, 0.0f);

            float modUV = (mFlipUVs.x) ? 1.0f - (uvs * (float) ix) : uvs * (float) ix;

            if (downBorder) mUVs.emplace_back(modUV, (mFlipUVs.y) ? 0.0f : 1.0f);
            if (!downBorder) mUVs.emplace_back(modUV, (mFlipUVs.y) ? 1.0f : 0.0f);
        }

        for (int ix = 0; ix < grid_x; ix++) {
            const float x = (float) ix * segment_x_length - length_x_half;

            mVertices.emplace_back(x, -1.0f, constraint);

            mNormals.emplace_back(0.0f, 1.0f, 0.0f);

            float modUV = (mFlipUVs.x) ? 1.0f - (uvs * (float) ix) : uvs * (float) ix;

            if (downBorder) mUVs.emplace_back(modUV, (mFlipUVs.y) ? -0.1f : 1.1f);
            if (!downBorder) mUVs.emplace_back(modUV, (mFlipUVs.y) ? 1.1f : -0.1f);
        }

        for (int ix = 0; ix < mSegments.x; ix++) {
            unsigned int a = ix;
            unsigned int b = ix + 1;
            unsigned int d = (ix + 1) + (mSegments.x + 1);
            unsigned int c = ix + (mSegments.x + 1);

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
            (const float &constraint, const bool &rightBorder) {
        auto heights = mElevation.getColumn((rightBorder) ? 0 : mSegments.x);

        const auto length_y = mDims.y;
        const int grid_y = mSegments.y + 1;
        const float length_y_half = length_y / 2.0f;
        const float segment_y_length = length_y / (float) mSegments.y;

        const auto vtxCountCopy = mVertices.size();

        const float uvs = 1.0f / (float) mSegments.y;
        for (int ix = 0; ix < grid_y; ix++) {
            const float x = (float) ix * segment_y_length - length_y_half;

            const float val = heights[heights.size() - ix - 1];
            const float height = (val <= 5000.0f) ? val * 0.001f : 0.0f;

            mVertices.emplace_back(constraint, height, -x);

            mNormals.emplace_back(0.0f, 1.0f, 0.0f);

            float modUV = (!mFlipUVs.y) ? 1.0f - (uvs * (float) ix) : uvs * (float) ix;

            if (rightBorder) mUVs.emplace_back((mFlipUVs.x) ? 0.0f : 1.0f, modUV);
            if (!rightBorder) mUVs.emplace_back((mFlipUVs.x) ? 1.0f : 0.0f, modUV);
        }

        for (int ix = 0; ix < grid_y; ix++) {
            const float x = (float) ix * segment_y_length - length_y_half;

            mVertices.emplace_back(constraint, -1.0f, -x);

            mNormals.emplace_back(0.0f, 1.0f, 0.0f);

            float modUV = (!mFlipUVs.y) ? 1.0f - (uvs * (float) ix) : uvs * (float) ix;

            if (rightBorder) mUVs.emplace_back((mFlipUVs.x) ? -0.1f : 1.1f, modUV);
            if (!rightBorder) mUVs.emplace_back((mFlipUVs.x) ? 1.1f : -0.1f, modUV);
        }

        for (int ix = 0; ix < mSegments.y; ix++) {
            unsigned int a = ix;
            unsigned int b = ix + 1;
            unsigned int d = (ix + 1) + (mSegments.y + 1);
            unsigned int c = ix + (mSegments.y + 1);

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