//
// Created by anshu on 8/18/2022.
//
#include "LayerEvent.hpp"

namespace KCore {
    LayerEvent LayerEvent::MakeInFrustumEvent
            (const std::string &quadcode, const TileDescription &description) {
        LayerEvent event{.type = InFrustum, .payload = new TilePayloadEvent(description)};
#if defined(_MSC_VER)
        strcpy_s(event.quadcode, quadcode.c_str());
#elif defined(__GNUC__)
        strcpy(event.quadcode, quadcode.c_str());
#endif
        return event;
    }

    LayerEvent LayerEvent::MakeNotInFrustumEvent
            (const std::string &quadcode) {
        LayerEvent event{.type = NotInFrustum, .payload = nullptr};
#if defined(_MSC_VER)
        strcpy_s(event.quadcode, quadcode.c_str());
#elif defined(__GNUC__)
        strcpy(event.quadcode, quadcode.c_str());
#endif
        return event;
    }

    LayerEvent LayerEvent::MakeImageEvent
            (const std::string &quadcode, const std::vector<uint8_t> &result) {
        LayerEvent event{.type = ImageReady, .payload = new ImagePayloadEvent(result)};
#if defined(_MSC_VER)
        strcpy_s(event.quadcode, quadcode.c_str());
#elif defined(__GNUC__)
        strcpy(event.quadcode, quadcode.c_str());
#endif
        return event;
    }
}