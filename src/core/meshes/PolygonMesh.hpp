#pragma once

#include "BaseMesh.hpp"

#include <vector>
#include <array>
#include <stdexcept>
#include <iostream>

#include <mapbox/earcut.hpp>

#include "../sources/local/geojson/GeoJSONObject.hpp"

#include <glm/gtx/normal.hpp>

namespace KCore {
    class PolygonMesh : public BaseMesh {
    public:
        PolygonMesh(const GeoJSONObject &object,
                    const std::vector<std::array<double, 2>> &convertedMainCoords,
                    const std::vector<std::array<double, 2>> &convertedHoleCoords) {
            if (object.mType != Polygon && object.mType != PolygonWithHole)
                throw std::runtime_error("Can't instantiate mesh using object of different type!");

//            createMeshFromObject(object, convertedMainCoords, convertedHoleCoords);

            createMeshFromObjectAndExtrude(object, convertedMainCoords, convertedHoleCoords);
        }

    private:
        void createMeshFromObjectAndExtrude(const GeoJSONObject &object,
                                            const std::vector<std::array<double, 2>> &convertedMainCoords,
                                            const std::vector<std::array<double, 2>> &convertedHoleCoords) {
            auto withHole = !convertedHoleCoords.empty();

            std::vector<std::vector<std::array<double, 2>>> collector{{},
                                                                      {}};
            collector[0].insert(collector[0].begin(), convertedMainCoords.begin(), convertedMainCoords.end() - 1);
            if (withHole)
                collector[1].insert(collector[1].begin(), convertedHoleCoords.begin(), convertedHoleCoords.end() - 1);

            auto indices = mapbox::earcut<uint32_t>(collector);
            for (int i = 0; i < indices.size(); i += 3) {
                auto &a = indices[i + 0];
                auto &b = indices[i + 1];
                auto &c = indices[i + 2];
                mIndices.insert(mIndices.end(), {c, b, a});
            }

            for (const auto &constant: std::vector<float>{150.0f, 0.0f}) {
                for (const auto &item: collector[0]) {
                    mVertices.insert(mVertices.end(), {item[0], constant, item[1]});
                    mUVs.insert(mUVs.end(), {0.0f, 0.0f});
                }

                for (const auto &item: collector[1]) {
                    mVertices.insert(mVertices.end(), {item[0], constant, item[1]});
                    mUVs.insert(mUVs.end(), {0.0f, 0.0f});
                }
            }

            for (int i = 0; i < collector[0].size(); i += 1) {
                uint32_t a, b, c, d;
                if (i == collector[0].size() - 1) {
                    a = (uint32_t) i;
                    b = (uint32_t) 0;
                    c = (uint32_t) (i + collector[0].size() + collector[1].size());
                    d = (uint32_t) (collector[0].size() + collector[1].size());
                } else {
                    a = (uint32_t) i + 0;
                    b = (uint32_t) i + 1;
                    c = (uint32_t) (i + 0 + collector[0].size() + collector[1].size());
                    d = (uint32_t) (i + 1 + collector[0].size() + collector[1].size());
                }

                mIndices.insert(mIndices.end(), {a, b, c});
                mIndices.insert(mIndices.end(), {c, b, d});
            }

            if (withHole) {
                for (int i = 0; i < collector[1].size(); i += 1) {
                    uint32_t a, b, c, d;
                    if (i == collector[1].size() - 1) {
                        a = (uint32_t) (i + collector[0].size());
                        b = (uint32_t) (collector[0].size());
                        c = (uint32_t) (i + (2 * collector[0].size()) + collector[1].size());
                        d = (uint32_t) ((2 * collector[0].size()) + collector[1].size());
                    } else {
                        a = (uint32_t) (i + 0 + collector[0].size());
                        b = (uint32_t) (i + 1 + collector[0].size());
                        c = (uint32_t) (i + 0 + (2 * collector[0].size()) + collector[1].size());
                        d = (uint32_t) (i + 1 + (2 * collector[0].size()) + collector[1].size());
                    }

                    mIndices.insert(mIndices.end(), {c, b, a});
                    mIndices.insert(mIndices.end(), {d, b, c});
                }
            }

//            std::map<uint32_t, glm::vec3> normals;
//            for (int i = 0; i < mIndices.size(); i += 3) {
//                const auto &a = mIndices[i + 0];
//                const auto &b = mIndices[i + 1];
//                const auto &c = mIndices[i + 2];
//
//                const auto &vtxA = glm::normalize(mVertices[a]);
//                const auto &vtxB = glm::normalize(mVertices[b]);
//                const auto &vtxC = glm::normalize(mVertices[c]);
//
//
//            }

            mNormals = std::vector<glm::vec3>(mVertices.size());

            const std::function<glm::vec3(glm::vec3, glm::vec3, glm::vec3)> computeFaceNormal =
                    [](glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
                        auto a = p3 - p2;
                        auto b = p1 - p2;
                        return glm::normalize(glm::cross(a, b));
                    };

            for (unsigned int i = 0; i < mIndices.size(); i += 3) {
                glm::vec3 A = mVertices[mIndices[i]];
                glm::vec3 B = mVertices[mIndices[i + 1]];
                glm::vec3 C = mVertices[mIndices[i + 2]];
                glm::vec3 normal = computeFaceNormal(A, B, C);

                mNormals[mIndices[i]] += normal;
                mNormals[mIndices[i + 1]] += normal;
                mNormals[mIndices[i + 2]] += normal;
            }
        }

        void createMeshFromObject(const GeoJSONObject &object,
                                  const std::vector<std::array<double, 2>> &convertedMainCoords,
                                  const std::vector<std::array<double, 2>> &convertedHoleCoords) {
            std::vector<std::vector<std::array<double, 2>>> collector;
            collector.push_back(convertedMainCoords);
            collector.push_back(convertedHoleCoords);

            auto indices = mapbox::earcut<uint32_t>(collector);
            for (int i = 0; i < indices.size(); i += 3) {
                auto &a = indices[i + 0];
                auto &b = indices[i + 1];
                auto &c = indices[i + 2];
                mIndices.insert(mIndices.end(), {c, b, a});
            }

            for (const auto &item: collector[0]) {
                mVertices.insert(mVertices.end(), {item[0], 0.0f, item[1]});
                mNormals.insert(mNormals.end(), {0.0f, 1.0f, 0.0f});
                mUVs.insert(mUVs.end(), {0.0f, 0.0f});
            }

            for (const auto &item: collector[1]) {
                mVertices.insert(mVertices.end(), {item[0], 0.0f, item[1]});
                mNormals.insert(mNormals.end(), {0.0f, 1.0f, 0.0f});
                mUVs.insert(mUVs.end(), {0.0f, 0.0f});
            }
        }

        void createMesh()
        override {

        }
    };
}
