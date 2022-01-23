#pragma once

#include "BaseWorld.hpp"

namespace KCore {
    class PlainWorld : public BaseWorld {
    public:
        PlainWorld(const glm::vec2 &originLatLon, const glm::vec2 &originPoint,
                   const struct WorldConfig &config) : BaseWorld(originLatLon, originPoint, config) {}

        void update() override {
            BaseWorld::update();
        }

    private:
        void calculateTiles() override {
            BaseWorld::calculateTiles();
        }
    };
}