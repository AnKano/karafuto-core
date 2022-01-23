#pragma once

#include "BaseMesh.hpp"

#include <vector>
#include <array>

#include <mapbox/earcut.hpp>

namespace KCore {
    class PolygonMesh : public BaseMesh {
    public:
        void createMesh() override {
            std::vector<std::vector<std::array<double, 2>>> polygon;
            polygon.push_back({{100, 0}, {100, 100}, {0, 100}, {0, 0}});
            polygon.push_back({{75, 25}, {75, 75}, {25, 75}, {25, 25}});
            std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(polygon);
        }
    };
}
