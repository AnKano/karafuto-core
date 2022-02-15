#pragma once

#include "BaseWorld.hpp"

#include "../../bindings.hpp"

namespace KCore {
    class PlainWorld : public BaseWorld {
    public:
        PlainWorld() : BaseWorld(0.0f, 0.0f) {}

        PlainWorld(float latitude, float longitude) : BaseWorld(latitude, longitude) {}

        void makeEvents() override;

    private:
        void calculateTiles() override;
    };

    extern "C" {
    DllExport KCore::PlainWorld *CreatePlainWorld(float latitude, float longitude);
    }
}