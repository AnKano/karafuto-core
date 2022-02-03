#pragma once

#include "../geography/TileDescription.hpp"

namespace KCore {
    enum EventType {
        InFrustum = 0,
        NotInFrustum,

        ContentLoadedRender,
        ContentLoadedImage
    };

    struct MapEvent {
        EventType type;
        char quadcode[32];
        void *payload;

        static MapEvent MakeInFrustumEvent(const std::string &quadcode, void* payloadPtr) {
            MapEvent event{};
            event.type = InFrustum;
            event.payload = payloadPtr;
            strcpy_s(event.quadcode, quadcode.c_str());

            return event;
        }

        static MapEvent MakeNotInFrustumEvent(const std::string &quadcode) {
            MapEvent event{};
            event.type = NotInFrustum;
            event.payload = nullptr;
            strcpy_s(event.quadcode, quadcode.c_str());

            return event;
        }

        static MapEvent MakeRenderLoadedEvent(const std::string &quadcode) {
            MapEvent event{};
            event.type = ContentLoadedRender;
            event.payload = nullptr;
            strcpy_s(event.quadcode, quadcode.c_str());

            return event;
        }

        static MapEvent MakeImageLoadedEvent(const std::string &quadcode, void* payloadPtr) {
            MapEvent event{};
            event.type = ContentLoadedImage;
            strcpy_s(event.quadcode, quadcode.c_str());
            event.payload = payloadPtr;

            return event;
        }
    };
}