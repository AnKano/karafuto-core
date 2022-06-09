#pragma once

#include "BaseMesh.hpp"

#include <stdexcept>
#include <iostream>

#include <glm/gtx/vector_angle.hpp>
#include <mapbox/earcut.hpp>

#include "../../sources/local/geojson/GeoJSONObject.hpp"

namespace KCore {
    class PolylineMesh : public BaseMesh {
    public:
        PolylineMesh(const GeoJSONObject &object, const std::vector<std::array<double, 2>> &convertedCoords) {
            if (object.mType != Polyline)
                throw std::runtime_error("Can't instantiate mesh using object of different type!");

            createMeshFromObject(object, convertedCoords);
        }

    private:
        void
        createMeshFromObject(const GeoJSONObject &object,
                             const std::vector<std::array<double, 2>> &convertedCoords) {
            std::vector<std::vector<std::array<double, 2>>> collector;
            // map main shape
            collector.emplace_back();
            // map hole shape (we should avoid it after)
            collector.emplace_back();

            std::vector<std::array<double, 2>> forwardCollector, reverseCollector;

            // !TODO: remove to parameter
            const auto modifier = 50.0f;

            bool isStart = true;
            glm::vec2 prevDirection;
            for (int i = 0; i < convertedCoords.size() - 1; i++) {
                auto firstPoint = glm::vec2{convertedCoords[i][0], convertedCoords[i][1]};
                auto secondPoint = glm::vec2{convertedCoords[i + 1][0], convertedCoords[i + 1][1]};

                auto direction = glm::normalize(secondPoint - firstPoint);
                auto clockwisePerp = glm::normalize(glm::vec2{direction.y, -direction.x});
                auto counterClockwisePerp = glm::normalize(glm::vec2{-direction.y, direction.x});

//                std::cout.precision(17);

                auto aPl = firstPoint + counterClockwisePerp * modifier;
                auto aPr = firstPoint + clockwisePerp * modifier;
                auto bPl = secondPoint + counterClockwisePerp * modifier;
                auto bPr = secondPoint + clockwisePerp * modifier;

                forwardCollector.push_back({aPl.x, aPl.y});
                forwardCollector.push_back({bPl.x, bPl.y});
                reverseCollector.push_back({aPr.x, aPr.y});
                reverseCollector.push_back({bPr.x, bPr.y});

                if (isStart) {
                    prevDirection = direction;
                    isStart = false;
                    continue;
                }

                auto clockwiseAngle = std::atan2(
                        direction.x * prevDirection.y - direction.y * prevDirection.x,
                        direction.x * prevDirection.x + direction.y * prevDirection.y
                );

                // isTurnRight
                if (clockwiseAngle > 0.0f) {
                    auto &leanPoint1 = forwardCollector[forwardCollector.size() - 3];
                    auto &leanPoint2 = forwardCollector[forwardCollector.size() - 2];

                    auto dirToPoint1 = glm::normalize(glm::vec2{leanPoint1[0], leanPoint1[1]} - firstPoint);
                    auto dirToPoint2 = glm::normalize(glm::vec2{leanPoint2[0], leanPoint2[1]} - firstPoint);

                    auto dirSum = -glm::normalize(dirToPoint1 + dirToPoint2);

                    auto patch = firstPoint + (dirSum * modifier);

                    reverseCollector.erase(reverseCollector.end() - 3, reverseCollector.end() - 1);
                    reverseCollector.insert(reverseCollector.end() - 1, {patch.x, patch.y});
                } else {
                    auto &leanPoint1 = reverseCollector[reverseCollector.size() - 3];
                    auto &leanPoint2 = reverseCollector[reverseCollector.size() - 2];

                    auto dirToPoint1 = glm::normalize(glm::vec2{leanPoint1[0], leanPoint1[1]} - firstPoint);
                    auto dirToPoint2 = glm::normalize(glm::vec2{leanPoint2[0], leanPoint2[1]} - firstPoint);

                    auto dirSum = -glm::normalize(dirToPoint1 + dirToPoint2);

                    auto patch = firstPoint + (dirSum * modifier);

                    forwardCollector.erase(forwardCollector.end() - 3, forwardCollector.end() - 1);
                    forwardCollector.insert(forwardCollector.end() - 1, {patch.x, patch.y});
                }

                prevDirection = direction;
            }

            collector[0] = forwardCollector;
            collector[0].insert(collector[0].end(), reverseCollector.rbegin(), reverseCollector.rend());

//            for (const auto &item: forwardCollector) {
//                std::cout << "(" << item[0] << ", " << item[1] << ")" << std::endl;
//            }
//
//            for (const auto &item: reverseCollector) {
//                std::cout << "(" << item[0] << ", " << item[1] << ")" << std::endl;
//            }
//
//            std::cout << std::endl;
//
//            for (const auto &item: object.mMainShapeCoords) {
//                std::cout << "(" << item[0] << ", " << item[1] << ")" << std::endl;
//            }
//
//            for (const auto &item: collector[0]) {
//                std::cout << "(" << item[0] << ", " << item[1] << ")" << std::endl;
//            }

            for (const auto &item: collector[0]) {
                mVertices.insert(mVertices.end(), {item[0], 0.0f, item[1]});
                mNormals.insert(mNormals.end(), {0.0f, 1.0f, 0.0f});
                mUVs.insert(mUVs.end(), {0.0f, 0.0f});
            }

            auto indices = mapbox::earcut<uint32_t>(collector);
            for (int i = 0; i < indices.size(); i += 3) {
                auto &a = indices[i + 0];
                auto &b = indices[i + 1];
                auto &c = indices[i + 2];
                mIndices.insert(mIndices.end(), {c, b, a});
            }
        }

        void createMesh() override {

        }
    };
}
