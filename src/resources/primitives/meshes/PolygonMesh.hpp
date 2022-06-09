#pragma once

#include "BaseMesh.hpp"

#include <vector>
#include <array>
#include <stdexcept>
#include <iostream>

#include "mapbox/earcut.hpp"

#include "../GeoJSONObject.hpp"

#include "glm/gtx/normal.hpp"

namespace KCore {
    class PolygonMesh : public BaseMesh {
    private:
        enum ShapeOrientation {
            CW, CCW, NONE
        };

    public:
        PolygonMesh(const GeoJSONObject &object,
                    const std::vector<std::array<double, 2>> &convertedMainCoords,
                    const std::vector<std::array<double, 2>> &convertedHoleCoords) : BaseMesh() {
            if (object.mType != Polygon && object.mType != PolygonWithHole)
                throw std::runtime_error("Can't instantiate mesh using object of different type!");

//            createMeshFromObject(object, convertedMainCoords, convertedHoleCoords);
            createMeshFromObjectAndExtrude(object, convertedMainCoords, convertedHoleCoords);
        }

    private:
        static ShapeOrientation calculateShapeOrientation(const std::vector<std::array<double, 2>> &data) {
            double accumulator = 0;

            for (int i = 0; i < data.size() - 1; i++) {
                auto &point1 = data[i];
                auto &point2 = data[i + 1];

                auto x1 = point1[0];
                auto x2 = point2[0];

                auto y1 = point1[1];
                auto y2 = point2[1];

                auto result = (x2 - x1) * (y2 + y1);
                accumulator += result;
            }

            return (accumulator > 0 ? CW : CCW);
        }

        void createMeshFromObjectAndExtrude(const GeoJSONObject &object,
                                            const std::vector<std::array<double, 2>> &convertedMainCoords,
                                            const std::vector<std::array<double, 2>> &convertedHoleCoords) {
            auto withHole = !convertedHoleCoords.empty();

            std::vector<std::vector<std::array<double, 2>>> collector;
            collector.push_back(convertedMainCoords); // collector for main shape
            collector.push_back(convertedHoleCoords); // collector for hole shape

            auto indices = mapbox::earcut<uint32_t>(collector);
            for (int i = 0; i < indices.size(); i += 3) {
                auto &a = indices[i + 0];
                auto &b = indices[i + 1];
                auto &c = indices[i + 2];
                mIndices.insert(mIndices.end(), {c, b, a});
            }

            for (const auto &constant: std::vector<float>{150.0f}) {
                for (const auto &item: collector[0]) {
                    mVertices.insert(mVertices.end(), {item[0], constant, item[1]});
                    mUVs.insert(mUVs.end(), {0.0f, 0.0f});
                }

                for (const auto &item: collector[1]) {
                    mVertices.insert(mVertices.end(), {item[0], constant, item[1]});
                    mUVs.insert(mUVs.end(), {0.0f, 0.0f});
                }
            }

            uint32_t lastIdx = mVertices.size();

            ShapeOrientation mainShapeOrient = calculateShapeOrientation(convertedMainCoords);

            for (int i = 0; i < collector[0].size(); i++) {
                std::array<double, 2> a{};
                std::array<double, 2> b{};

                if (collector[0].size() - 1 == i) {
                    a = collector[0][i];
                    b = collector[0][0];
                } else {
                    a = collector[0][i];
                    b = collector[0][i + 1];
                }

                auto ptA = glm::vec3{a[0], 150.0f, a[1]};
                auto ptB = glm::vec3{b[0], 150.0f, b[1]};
                auto ptC = glm::vec3{a[0], 0.0f, a[1]};
                auto ptD = glm::vec3{b[0], 0.0f, b[1]};

                mVertices.insert(mVertices.end(), ptA);
                mVertices.insert(mVertices.end(), ptB);
                mVertices.insert(mVertices.end(), ptC);
                mVertices.insert(mVertices.end(), ptD);

                for (int j = 0; j < 4; j++)
                    mUVs.insert(mUVs.end(), {0.0f, 0.0f});

                if (mainShapeOrient == CCW) {
                    mIndices.insert(mIndices.end(), {lastIdx + 0, lastIdx + 1, lastIdx + 2});
                    mIndices.insert(mIndices.end(), {lastIdx + 2, lastIdx + 1, lastIdx + 3});
                } else if (mainShapeOrient == CW) {
                    mIndices.insert(mIndices.end(), {lastIdx + 2, lastIdx + 1, lastIdx + 0});
                    mIndices.insert(mIndices.end(), {lastIdx + 3, lastIdx + 1, lastIdx + 2});
                }

                lastIdx += 4;
            }

            if (withHole) {
                ShapeOrientation holeShapeOrient = calculateShapeOrientation(convertedHoleCoords);

                for (int i = 0; i < collector[1].size(); i++) {
                    std::array<double, 2> a{};
                    std::array<double, 2> b{};

                    if (collector[1].size() - 1 == i) {
                        a = collector[1][i];
                        b = collector[1][0];
                    } else {
                        a = collector[1][i];
                        b = collector[1][i + 1];
                    }

                    auto ptA = glm::vec3{a[0], 150.0f, a[1]};
                    auto ptB = glm::vec3{b[0], 150.0f, b[1]};
                    auto ptC = glm::vec3{a[0], 0.0f, a[1]};
                    auto ptD = glm::vec3{b[0], 0.0f, b[1]};

                    mVertices.insert(mVertices.end(), ptA);
                    mVertices.insert(mVertices.end(), ptB);
                    mVertices.insert(mVertices.end(), ptC);
                    mVertices.insert(mVertices.end(), ptD);

                    for (int j = 0; j < 4; j++)
                        mUVs.insert(mUVs.end(), {0.0f, 0.0f});

                    if (holeShapeOrient == CCW) {
                        mIndices.insert(mIndices.end(), {lastIdx + 2, lastIdx + 1, lastIdx + 0});
                        mIndices.insert(mIndices.end(), {lastIdx + 3, lastIdx + 1, lastIdx + 2});
                    } else if (holeShapeOrient == CW) {
                        mIndices.insert(mIndices.end(), {lastIdx + 0, lastIdx + 1, lastIdx + 2});
                        mIndices.insert(mIndices.end(), {lastIdx + 2, lastIdx + 1, lastIdx + 3});
                    }

                    lastIdx += 4;
                }
            }

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
            collector.push_back(convertedMainCoords); // collector for main shape
            collector.push_back(convertedHoleCoords); // collector for hole shape

            auto indices = mapbox::earcut<uint32_t>(collector);
            for (int i = 0; i < indices.size(); i += 3) {
                auto &a = indices[i + 0];
                auto &b = indices[i + 1];
                auto &c = indices[i + 2];
                mIndices.insert(mIndices.end(), {c, b, a});
            }

            for (const auto &constant: std::vector<float>{0.0f}) {
                for (const auto &item: collector[0]) {
                    mVertices.insert(mVertices.end(), {item[0], constant, item[1]});
                    mUVs.insert(mUVs.end(), {0.0f, 0.0f});
                }

                for (const auto &item: collector[1]) {
                    mVertices.insert(mVertices.end(), {item[0], constant, item[1]});
                    mUVs.insert(mUVs.end(), {0.0f, 0.0f});
                }
            }
        }

        void createMesh() override {

        }
    };
}
