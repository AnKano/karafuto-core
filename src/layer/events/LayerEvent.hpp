#pragma once

#include <cstring>

#include "../../geography/TileDescription.hpp"
#include "../../layer/presenters/ImageResult.hpp"

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

        static LayerEvent MakeInFrustumEvent(const std::string &quadcode, TilePayload* payloadPtr) {
            LayerEvent event{};
            event.type = InFrustum;
            event.payload = payloadPtr;
            std::strcpy(event.quadcode, quadcode.c_str());

            return event;
        }

        static LayerEvent MakeNotInFrustumEvent(const std::string &quadcode) {
            LayerEvent event{};
            event.type = NotInFrustum;
            event.payload = nullptr;
            std::strcpy(event.quadcode, quadcode.c_str());

            return event;
        }

        static LayerEvent MakeImageEvent(const std::string &quadcode, ImageResult* payloadPtr) {
            LayerEvent event{};
            event.type = ImageReady;
            event.payload = payloadPtr;
            std::strcpy(event.quadcode, quadcode.c_str());

            return event;
        }

//        static LayerEvent MakeTerrainEvent(const std::string &quadcode, void* payloadPtr) {
//            LayerEvent event{};
//            event.type = TerrainLoaded;
//            event.payload = payloadPtr;
//            std::strcpy(event.quadcode, quadcode.c_str());
//
//            return event;
//        }
//
//        static LayerEvent MakeGeoJSONEvent(const std::string &quadcode, void* payloadPtr) {
//            LayerEvent event{};
//            event.type = GeoJSONLoaded;
//            event.payload = payloadPtr;
//            std::strcpy(event.quadcode, quadcode.c_str());
//
//            return event;
//        }
    };
}