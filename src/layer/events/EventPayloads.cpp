
#include "EventPayloads.hpp"

namespace KCore {
    ImagePayloadEvent::ImagePayloadEvent
            (const std::vector<uint8_t> &rawResult) {
        int w = -1, h = -1, ch = -1;
        auto result = STBImageUtils::decodeImageBuffer(rawResult.data(), rawResult.size(), w, h, ch);

        setWidthHeight(w, h);
        setFormat(ch);
        setData(result);
    }

    void ImagePayloadEvent::setWidthHeight
            (const int &w, const int &h) {
        width = w;
        height = h;
    }

    void ImagePayloadEvent::setFormat
            (const int &channels) {
        switch (channels) {
            case 2:
                format = RGB565;
                break;
            case 3:
                format = RGB888;
                break;
            case 4:
                format = RGBA8888;
                break;
            default:
                throw std::runtime_error("Unable to find a suitable image format!");
        }
    }

    void ImagePayloadEvent::setData
            (const std::vector<uint8_t> &result) {
        size = result.size();
        data = new uint8_t[size];
        std::copy(result.begin(), result.end(), data);
    }

    TilePayloadEvent::TilePayloadEvent
            (const TileDescription &description) {
        setTilecode(description);
        setCenter(description);
        setScale(description);
        setType(description);
        setVisibility(description);
        setQuadcode(description);
    }

    void TilePayloadEvent::setTilecode
            (const TileDescription &description) {
        tilecode = description.getTilecode();
    }

    void TilePayloadEvent::setCenter
            (const TileDescription &description) {
        center = description.getCenter();
    }

    void TilePayloadEvent::setScale
            (const TileDescription &description) {
        scale = description.getScale();
    }

    void TilePayloadEvent::setType
            (const TileDescription &description) {
        type = description.getType();
    }

    void TilePayloadEvent::setVisibility
            (const TileDescription &description) {
        visibility = description.getVisibility();
    }

    void TilePayloadEvent::setQuadcode
            (const TileDescription &description) {
#if defined(_MSC_VER)
        strcpy_s(quadcode, description.getQuadcode().c_str());
#elif defined(__GNUC__)
        strcpy(quadcode, description.getQuadcode().c_str());
#endif
    }
}