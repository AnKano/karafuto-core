#pragma once

#include <cstdint>
#include <vector>

#include "../../misc/STBImageUtils.hpp"
#include "../../geography/TileDescription.hpp"

namespace KCore {
    enum ImageFormat {
        RGB565 = 0,
        RGB888 = 1,
        RGBA8888 = 2
    };

    struct ImagePayloadEvent {
        /* 00..04..08     bytes */
        uint32_t width{0}, height{0};
        /* 08..12         bytes */
        ImageFormat format{};
        /* 12..20         bytes */
        uint64_t size{0};
        /* 20..28         bytes */
        uint8_t *data{nullptr};

    public:
        explicit ImagePayloadEvent
                (const std::vector<uint8_t> &rawResult);

    private:
        void setWidthHeight
                (const int &w, const int &h);

        void setFormat
                (const int &channels);

        void setData
                (const std::vector<uint8_t> &result);
    };

    struct TilePayloadEvent {
        /* 00..04..08..12 bytes */
        glm::ivec3 tilecode{0, 0, 0};
        /* 12..16..20     bytes */
        glm::vec2 center{0.0f, 0.0f};
        /* 20..24         bytes */
        float scale{0.0f};
        /* 24..28         bytes */
        TileType type{Leaf};
        /* 28..32         bytes */
        TileVisibility visibility{Visible};
        /* 32..64         bytes */
        char quadcode[32]{};

    public:
        explicit TilePayloadEvent
                (const TileDescription &description);

    private:
        void setTilecode
                (const TileDescription &description);

        void setCenter
                (const TileDescription &description);

        void setScale
                (const TileDescription &description);

        void setType
                (const TileDescription &description);

        void setVisibility
                (const TileDescription &description);

        void setQuadcode
                (const TileDescription &description);
    };
}