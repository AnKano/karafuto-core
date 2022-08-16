#pragma once

#include "glm/glm.hpp"

#ifdef WIN32

#include <cmath>

#endif

#include <string>
#include <iostream>

namespace KCore {
    class GeographyConverter {
    public:
        constexpr static float R = 6378137.0f;
        constexpr static float MAX_LATITUDE = 85.0511287798f;

        constexpr static float MULTIPLIER = 0.0001f;
        constexpr static float MESH_MULTIPLIER = 0.001f;

        static glm::vec2 project
                (const glm::vec2 &latLon) {
            auto max = MAX_LATITUDE;
            auto lat = fmax(fmin(max, latLon.x), -max);

            return {
                    R * latLon.y * (M_PI / 180.0f),
                    R * log(tan(M_PI / 4.0f + (lat * (M_PI / 180.0f)) / 2.0f))
            };
        }

        static glm::vec2 unproject
                (const glm::vec2 &point) {
            return {
                    180.0f * (2.0f * atanf(expf(point.y / R)) - (M_PI / 2)) / M_PI,
                    point.x * 180.0f / M_PI / R
            };
        }

        static glm::vec2 latLonToPoint
                (const glm::vec2 &latLon) {
            auto projected = GeographyConverter::project(latLon);
            projected.x *= -1.0f;
            projected.y *= -1.0f;

            projected.x *= MULTIPLIER;
            projected.y *= MULTIPLIER;

            return projected;
        }

        static glm::vec2 pointToLatLon
                (const glm::vec2 &point) {
            auto _point = glm::vec2(point.x, point.y);
            _point.x *= -1.0f;
            _point.y *= -1.0f;

            _point.x /= MULTIPLIER;
            _point.y /= MULTIPLIER;

            return GeographyConverter::unproject(_point);
        }

        static glm::ivec3 quadcodeToTilecode
                (const std::string &quadcode) {
            int x = 0, y = 0, z = (int) quadcode.length();

            for (int i = z; i > 0; i--) {
                auto mask = 1 << (i - 1);

                auto ch = quadcode[z - i];
                int q = ch - '0';

                if (q == 1)
                    x |= mask;
                if (q == 2)
                    y |= mask;
                if (q == 3) {
                    x |= mask;
                    y |= mask;
                }
            }

            return {x, y, z};
        }

        static float tileToLon
                (uint16_t x, uint16_t z) {
            return (float) x / powf(2.0f, z) * 360.0f - 180.0f;
        }

        static float tileToLat
                (uint16_t y, uint16_t z) {
            float n = M_PI - 2.0f * M_PI * y / powf(2.0f, z);
            return 180.0f / (float) M_PI * (float) atanf(0.5f * (expf(n) - expf(-n)));
        }

        [[maybe_unused]] static int lonToTileX
                (double lon, int z) {
            return (int) (floor((lon + 180.0) / 360.0 * (1 << z)));
        }

        [[maybe_unused]] static int lat2TileY
                (double lat, int z) {
            double latrad = lat * M_PI / 180.0;
            return (int) (floor((1.0 - asinh(tan(latrad)) / M_PI) / 2.0 * (1 << z)));
        }
    };
}