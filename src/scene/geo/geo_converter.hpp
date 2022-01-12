//
// Created by Ash on 2/6/2021.
//

#ifndef A_SIMPLE_TRIANGLE_WINDOWS_GEOCONVERTER_HPP
#define A_SIMPLE_TRIANGLE_WINDOWS_GEOCONVERTER_HPP

#include "glm/glm.hpp"

#ifdef WIN32
    #include <cmath>
#endif

#include <string>
#include <iostream>

namespace kcore {
    class geo_converter {
        constexpr static float R = 6378137.0;
        constexpr static float MAX_LATITUDE = 85.0511287798;

        constexpr static double MULTIPLIER = 1;
    public:
        static glm::vec2 project(const glm::vec2 &latLon) {
            auto max = MAX_LATITUDE;
            auto lat = fmax(fmin(max, latLon.x), -max);

            return {R * latLon.y * (M_PI / 180.),
                    R * log(tan(M_PI / 4.0 + (lat * (M_PI / 180.0)) / 2.0))};
        }

        static glm::vec2 unproject(const glm::vec2 &point) {
            return {180.0 * (2.0 * atanf(expf(point.y / R)) - (M_PI / 2)) / M_PI,
                    point.x * 180.0 / M_PI / R};
        }

        static glm::vec2 lat_lon_to_point(const glm::vec2 &lat_lon) {
            auto projected = geo_converter::project(lat_lon);

            projected.y *= -1;

            projected.x *= 1;
            projected.y *= 1;

            return projected;
        }

        static glm::vec2 point_to_lat_lon(const glm::vec2 &point) {
            auto _point = glm::vec2(point.x, point.y);

            _point.x /= MULTIPLIER;
            _point.y /= MULTIPLIER;

            return geo_converter::unproject(_point);
        }

        static glm::ivec3 quadcode_to_tilecode(const std::string &quadcode) {
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

        static float tile_to_lon(float x, float z) {
            return x / powf(2.0, z) * 360.0f - 180.0f;
        }

        static float tile_to_lat(float y, float z) {
            float n = M_PI - 2 * M_PI * y / powf(2.0, z);
            return 180.0f / (float) M_PI * (float) atan(0.5 * (expf(n) - expf(-n)));
        }
    };
}

#endif //A_SIMPLE_TRIANGLE_WINDOWS_GEOCONVERTER_HPP
