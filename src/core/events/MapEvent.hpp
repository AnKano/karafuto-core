#pragma once

#include "../geography/TileDescription.hpp"

namespace KCore {
    enum EventType {
        // common tiles event
        InFrustum = 0,
        NotInFrustum = 1,

        ContentLoaded = 2,
        ContentRefreshed = 3,

        CompletelyDissolved = 4

        // meta tiles event
        // ???
    };

    struct MapEvent {
        EventType Type;
        char Quadcode[32];
        void* OptionalPayload;
    };
}