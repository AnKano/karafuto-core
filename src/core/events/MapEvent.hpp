#pragma once

#include "../geography/TileDescription.hpp"
#include <cstring>

namespace KCore {
    enum EventType {
        InFrustum = 0,
        NotInFrustum,

        ContentLoadedRender,
        ContentLoadedImage,

        TerrainLoaded
    };

    struct MapEvent {
        EventType type;
        char quadcode[32];
        void *payload;

        static MapEvent MakeInFrustumEvent(const std::string &quadcode, void* payloadPtr) {
            MapEvent event{};
            event.type = InFrustum;
            event.payload = payloadPtr;
#if defined(_MSC_VER)
            strcpy_s(event.quadcode, quadcode.c_str());
#endif
#if defined(__GNUC__)
            strcpy(event.quadcode, quadcode.c_str());
#endif

            return event;
        }

        static MapEvent MakeNotInFrustumEvent(const std::string &quadcode) {
            MapEvent event{};
            event.type = NotInFrustum;
            event.payload = nullptr;
#if defined(_MSC_VER)
            strcpy_s(event.quadcode, quadcode.c_str());
#endif
#if defined(__GNUC__)
            strcpy(event.quadcode, quadcode.c_str());
#endif

            return event;
        }

        static MapEvent MakeRenderLoadedEvent(const std::string &quadcode) {
            MapEvent event{};
            event.type = ContentLoadedRender;
            event.payload = nullptr;
#if defined(_MSC_VER)
            strcpy_s(event.quadcode, quadcode.c_str());
#endif
#if defined(__GNUC__)
            strcpy(event.quadcode, quadcode.c_str());
#endif

            return event;
        }

        static MapEvent MakeImageLoadedEvent(const std::string &quadcode, void* payloadPtr) {
            MapEvent event{};
            event.type = ContentLoadedImage;
            event.payload = payloadPtr;
#if defined(_MSC_VER)
            strcpy_s(event.quadcode, quadcode.c_str());
#endif
#if defined(__GNUC__)
            strcpy(event.quadcode, quadcode.c_str());
#endif

            return event;
        }

        static MapEvent MakeTerrainEvent(const std::string &quadcode, void* payloadPtr) {
            MapEvent event{};
            event.type = TerrainLoaded;
            event.payload = payloadPtr;
#if defined(_MSC_VER)
            strcpy_s(event.quadcode, quadcode.c_str());
#endif
#if defined(__GNUC__)
            strcpy(event.quadcode, quadcode.c_str());
#endif

            return event;
        }
    };
}