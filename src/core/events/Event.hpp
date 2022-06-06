#pragma once

#include "../geography/TileDescription.hpp"
#include "../contexts/rendering/ImageResult.hpp"
#include <cstring>

namespace KCore {
    enum EventType {
        InFrustum = 0,
        NotInFrustum = 1,
        ImageReady = 2,
        TerrainLoaded = 3,
        GeoJSONLoaded = 4
    };

    struct Event {
        EventType type;
        char quadcode[32];
        void *payload;

        static Event MakeInFrustumEvent(const std::string &quadcode, TilePayload* payloadPtr) {
            Event event{};
            event.type = InFrustum;
            event.payload = payloadPtr;
            std::strcpy(event.quadcode, quadcode.c_str());

            return event;
        }

        static Event MakeNotInFrustumEvent(const std::string &quadcode) {
            Event event{};
            event.type = NotInFrustum;
            event.payload = nullptr;
            std::strcpy(event.quadcode, quadcode.c_str());

            return event;
        }

        static Event MakeImageEvent(const std::string &quadcode, ImageResult* payloadPtr) {
            Event event{};
            event.type = ImageReady;
            event.payload = payloadPtr;
            std::strcpy(event.quadcode, quadcode.c_str());

            return event;
        }

        static Event MakeTerrainEvent(const std::string &quadcode, void* payloadPtr) {
            Event event{};
            event.type = TerrainLoaded;
            event.payload = payloadPtr;
            std::strcpy(event.quadcode, quadcode.c_str());

            return event;
        }

        static Event MakeGeoJSONEvent(const std::string &quadcode, void* payloadPtr) {
            Event event{};
            event.type = GeoJSONLoaded;
            event.payload = payloadPtr;
            std::strcpy(event.quadcode, quadcode.c_str());

            return event;
        }
    };
}