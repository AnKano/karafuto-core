#include <utility>

#include "GridMesh.hpp"
#include "../../../geography/GeographyConverter.hpp"

namespace KCore {
    GridMesh::GridMesh
            (const glm::vec2 &dims, const glm::ivec2 &segments,
             const glm::bvec2 &flipUVs, Elevation elevation)
            : BaseMesh(), mDims(dims), mSegments(segments),
              mFlipUVs(flipUVs), mElevation(std::move(elevation)) {
        buildUpSurface();
        produceNorthBorder();
        produceSouthBorder();
        produceEastBorder();
        produceWestBorder();
    }

    void GridMesh::produceNorthBorder() {
        // horizontal and original
        buildBorder(true, false);
    }

    void GridMesh::produceSouthBorder() {
        // horizontal but opposite
        buildBorder(true, true);
    }

    void GridMesh::produceEastBorder() {
        // vertical and original
        buildBorder(false, false);
    }

    void GridMesh::produceWestBorder() {
        // vertical but opposite
        buildBorder(false, true);
    }

    void GridMesh::buildBorder(const bool &horizontal, const bool &opposite) {
        std::vector<float> heights;
        float edge_length;
        int edge_segments;

        if (horizontal) {
            edge_length = mDims.x;
            edge_segments = mSegments.x;
            heights = mElevation.getRow((opposite) ? 0 : edge_segments);
        } else {
            edge_length = mDims.y;
            edge_segments = mSegments.y;
            heights = mElevation.getColumn((opposite) ? 0 : edge_segments);
        }

        float constraint = edge_length;
        if (opposite) constraint *= -1;

        float length_half = edge_length / 2.0f;
        float constraint_half = constraint / 2.0f;
        float segment_length = edge_length / (float) edge_segments;
        int segments_count = edge_segments + 1;
        float segment_uv_offset = 1.0f / (float) edge_segments;

        const int UV_CONST_COUNT = 2;
        float uv_variants[2][UV_CONST_COUNT] = {
                {0.0f,  1.0f},
                {-0.1f, 1.1f}
        };

        if (horizontal) {
            for (std::size_t j = 0; j < UV_CONST_COUNT; j++) {
                auto uv_constrains = uv_variants[j];

                for (int i = 0; i < segments_count; i++) {
                    glm::vec3 pos;
                    pos.x = (float) i * segment_length - length_half;
                    pos.y = ((j == 0) ? heights[i] : 0.0f) * KCore::GeographyConverter::MESH_MULTIPLIER;
                    pos.z = constraint_half;

                    glm::vec2 uv;
                    uv.x = (float) i * segment_uv_offset;
                    uv.x = mFlipUVs.x ? 1.0f - uv.x : uv.x;
                    uv.y = mFlipUVs.y ? uv_constrains[0] : uv_constrains[1];
                    if (!opposite) uv.y = mFlipUVs.y ? uv_constrains[1] : uv_constrains[0];

                    glm::vec3 normals{0.0f, 1.0f, 0.0f};

                    mPositions.push_back(pos);
                    mUVs.push_back(uv);
                    mNormals.push_back(normals);
                }
            }
        } else {
            for (std::size_t j = 0; j < UV_CONST_COUNT; j++) {
                auto uv_constrains = uv_variants[j];

                for (int i = 0; i < segments_count; i++) {
                    glm::vec3 pos;
                    pos.x = constraint_half;
                    pos.y = ((j == 0) ? heights[i] : 0.0f) * KCore::GeographyConverter::MESH_MULTIPLIER;
                    pos.z = (float) i * segment_length - length_half;

                    glm::vec2 uv;
                    uv.x = mFlipUVs.x ? uv_constrains[0] : uv_constrains[1];
                    if (!opposite) uv.x = mFlipUVs.x ? uv_constrains[1] : uv_constrains[0];
                    uv.y = (float) i * segment_uv_offset;
                    uv.y = mFlipUVs.x ? 1.0f - uv.y : uv.y;

                    glm::vec3 normals{0.0f, 1.0f, 0.0f};

                    mPositions.push_back(pos);
                    mUVs.push_back(uv);
                    mNormals.push_back(normals);
                }
            }
        }

        const int SIDES_COUNT = 2;
        connectLastVertices(segments_count * SIDES_COUNT, edge_segments, opposite);
    }

    void GridMesh::buildUpSurface() {
        const float length_x_half = mDims.x / 2.0f;
        const float length_y_half = mDims.y / 2.0f;

        const int loop_steps_x = mSegments.x + 1;
        const int loop_steps_y = mSegments.y + 1;

        const float segment_x_length = mDims.x / (float) mSegments.x;
        const float segment_y_length = mDims.y / (float) mSegments.y;

        for (int iy = 0; iy < loop_steps_y; iy++) {
            for (int ix = 0; ix < loop_steps_x; ix++) {
                const float x = (float) ix * segment_x_length - length_x_half;
                const float y = (float) iy * segment_y_length - length_y_half;

                const float height = mElevation.get(iy, ix) * KCore::GeographyConverter::MESH_MULTIPLIER;
                mPositions.emplace_back(x, height, y);
                mNormals.emplace_back(0.0f, 1.0f, 0.0f);

                float uv_x = (float) ix / ((float) mSegments.x);
                float uv_y = (float) iy / ((float) mSegments.y);

                float mod_uv_x = (mFlipUVs.x) ? 1.0f - uv_x : uv_x;
                float mod_uv_y = (mFlipUVs.y) ? 1.0f - uv_y : uv_y;

                mUVs.emplace_back(mod_uv_x, mod_uv_y);
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

    void GridMesh::connectLastVertices(const int &num, const int &segments, const bool &opposite) {
        auto vtxCountCopy = mPositions.size();
        vtxCountCopy -= num;

        for (int i = 0; i < segments; i++) {
            unsigned int a = i;
            unsigned int b = i + 1;
            unsigned int d = i + 1 + segments + 1;
            unsigned int c = i + segments + 1;

            a += vtxCountCopy;
            b += vtxCountCopy;
            c += vtxCountCopy;
            d += vtxCountCopy;

            if (opposite) {
                mIndices.insert(mIndices.end(), {a, b, d});
                mIndices.insert(mIndices.end(), {a, d, c});
            } else {
                mIndices.insert(mIndices.end(), {d, b, a});
                mIndices.insert(mIndices.end(), {c, d, a});
            }
        }
    }
}