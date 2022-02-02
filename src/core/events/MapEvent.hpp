#pragma once

#include "../geography/TileDescription.hpp"

namespace KCore {
    enum EventType {
        // common tiles event
        InFrustum = 0,
        NotInFrustum,

        ContentLoadedRender,
        ContentLoadedImage,
    };

    struct MapEvent {
        EventType Type;
        char Quadcode[32];
        void *OptionalPayload;
    };
}