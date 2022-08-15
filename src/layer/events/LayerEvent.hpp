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

        static LayerEvent MakeInFrustumEvent(const std::string &quadcode, const TileDescription &description) {
            LayerEvent event{.type = InFrustum, .payload = new TilePayloadEvent(description)};
#if defined(_MSC_VER)
            strcpy_s(event.quadcode, quadcode.c_str());
#elif defined(__GNUC__)
            strcpy(event.quadcode, quadcode.c_str());
#endif
            return event;
        }

        static LayerEvent MakeNotInFrustumEvent(const std::string &quadcode) {
            LayerEvent event{.type = NotInFrustum, .payload = nullptr};
#if defined(_MSC_VER)
            strcpy_s(event.quadcode, quadcode.c_str());
#elif defined(__GNUC__)
            strcpy(event.quadcode, quadcode.c_str());
#endif
            return event;
        }

        static LayerEvent MakeImageEvent(const std::string &quadcode, const std::vector<uint8_t> &result) {
            LayerEvent event{.type = ImageReady, .payload = new ImageResultEvent(result)};
#if defined(_MSC_VER)
            strcpy_s(event.quadcode, quadcode.c_str());
#elif defined(__GNUC__)
            strcpy(event.quadcode, quadcode.c_str());
#endif
            return event;
        }
    };
}