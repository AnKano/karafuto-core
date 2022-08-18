#pragma once

#include <cstring>

#include "../../geography/TileDescription.hpp"
#include "EventPayloads.hpp"

namespace KCore {
    enum LayerEventType {
        InFrustum = 0,
        NotInFrustum = 1,
        ImageReady = 2
    };

    struct LayerEvent {
        LayerEventType type;
        char quadcode[32];
        void *payload;

        static LayerEvent MakeInFrustumEvent
                (const std::string &quadcode, const TileDescription &description);

        static LayerEvent MakeNotInFrustumEvent
                (const std::string &quadcode);

        static LayerEvent MakeImageEvent
                (const std::string &quadcode, const std::vector<uint8_t> &result);
    };
}