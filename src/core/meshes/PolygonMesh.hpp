#pragma once

#include "BaseMesh.hpp"

#include <vector>
#include <array>
#include <stdexcept>
#include <iostream>

#include <mapbox/earcut.hpp>

#include "../sources/local/geojson/GeoJSONObject.hpp"

namespace KCore {
    class PolygonMesh : public BaseMesh {
    public:
        PolygonMesh(const GeoJSONObject &object,
                    const std::vector<std::array<double, 2>> &convertedMainCoords,
                    const std::vector<std::array<double, 2>> &convertedHoleCoords) {
            if (object.mType != Polygon && object.mType != PolygonWithHole)
                throw std::runtime_error("Can't instantiate mesh using object of different type!");

            createMeshFromObject(object, convertedMainCoords, convertedHoleCoords);
        }

    private:
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

        void createMesh() override {

        }
    };
}
