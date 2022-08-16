#pragma once

#include <cstdint>
#include <vector>

#include "../../misc/STBImageUtils.hpp"

namespace KCore {
    enum ImageFormat {
        RGB565 = 0,
        RGB888 = 1,
        RGBA8888 = 2
    };

    struct ImageResultEvent {
        /* 00..04..08     bytes */
        uint32_t width{0}, height{0};
        /* 08..12         bytes */
        ImageFormat format{};
        /* 12..20         bytes */
        uint64_t size{0};
        /* 20..28         bytes */
        uint8_t *data{nullptr};

    public:
        explicit ImageResultEvent
                (const std::vector<uint8_t> &rawResult) {
            int w = -1, h = -1, ch = -1;
            auto result = STBImageUtils::decodeImageBuffer(rawResult.data(), rawResult.size(), w, h, ch);

            setWidthHeight(w, h);
            setFormat(ch);
            setData(result);
        }

    private:
        void setWidthHeight
                (const int &w, const int &h) {
            width = w;
            height = h;
        }

        void setFormat
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

        void setData
                (const std::vector<uint8_t> &result) {
            size = result.size();
            data = new uint8_t[size];
            std::copy(result.begin(), result.end(), data);
        }
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
                (const TileDescription &description) {
            setTilecode(description);
            setCenter(description);
            setScale(description);
            setType(description);
            setVisibility(description);
            setQuadcode(description);
        }

    private:
        void setTilecode
                (const TileDescription &description) {
            tilecode = description.getTilecode();
        }

        void setCenter
                (const TileDescription &description) {
            center = description.getCenter();
        }

        void setScale
                (const TileDescription &description) {
            scale = description.getScale();
        }

        void setType
                (const TileDescription &description) {
            type = description.getType();
        }

        void setVisibility
                (const TileDescription &description) {
            visibility = description.getVisibility();
        }

        void setQuadcode
                (const TileDescription &description) {
#if defined(_MSC_VER)
            strcpy_s(quadcode, description.getQuadcode().c_str());
#elif defined(__GNUC__)
            strcpy(quadcode, description.getQuadcode().c_str());
#endif
        }
    };
}