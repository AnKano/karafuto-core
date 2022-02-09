#pragma once

#include <vector>
#include <array>
#include <any>
#include <map>
#include <string>

#include <rapidjson/document.h>

namespace KCore {
    enum GeoJSONObjectType {
        Point = 0,
        Polyline,
        Polygon,
        PolygonWithHole
    };

    struct GeoJSONObject {
    public:
        GeoJSONObjectType mType;
        // 1 - main shape coords, 2 - hole shape coords
        std::vector<std::array<double, 2>> mMainShapeCoords;
        std::vector<std::array<double, 2>> mHoleShapeCoords;
        std::map<std::string, std::any> mProperties;

    public:
        GeoJSONObject(const rapidjson::Value &value) {
            restoreFromString(value);
        }

    private:
        void restoreFromString(const rapidjson::Value &value) {
            auto &geometry = value["geometry"];

            if (!geometry.IsObject()) throw std::runtime_error("Undefined file structure!");

            // get type of geometry
            mType = getType(geometry["type"]);

            // get coords
            auto mCoords = getCoords(geometry["coordinates"]);

            mMainShapeCoords = mCoords[0];
            mHoleShapeCoords = mCoords[1];

            //!TODO: add properties parsing
        }

        static GeoJSONObjectType getType(const rapidjson::Value &value) {
            if (!value.IsString()) throw std::runtime_error("Undefined file structure!");

            auto str = std::string{value.GetString()};
            if (str == "Point") return Point;
            if (str == "LineString") return Polyline;
            if (str == "Polygon") return Polygon;

            throw std::runtime_error("Undefined type of feature!");
        }

        std::array<std::vector<std::array<double, 2>>, 2> getCoords(const rapidjson::Value &value) {
            std::array<std::vector<std::array<double, 2>>, 2> coordsCollector;

            if (!value.IsArray()) throw std::runtime_error("Undefined file structure!");

            // parse as point
            if (mType == Point) {
                if (value.Size() != 2) throw std::runtime_error("Undefined file structure!");
                coordsCollector[0].push_back({value[0].GetDouble(), value[1].GetDouble()});
            }

            if (mType == Polyline) {
                for (const auto &nestedValue: value.GetArray()) {
                    if (nestedValue.Size() != 2) throw std::runtime_error("Undefined file structure!");
                    coordsCollector[0].push_back({nestedValue[0].GetDouble(), nestedValue[1].GetDouble()});
                }
            }

            if (mType == Polygon) {
                auto shapesCount = value.Size();
                if (!value.IsArray() || shapesCount > 2) throw std::runtime_error("Undefined file structure!");

                if (shapesCount == 2) mType = PolygonWithHole;

                for (int shape = 0; shape < shapesCount; shape++) {
                    const auto &shapeCoords = value[shape].GetArray();
                    for (const auto &coord: shapeCoords) {
                        if (coord.Size() != 2) throw std::runtime_error("Undefined file structure!");
                        coordsCollector[shape].push_back({coord[0].GetDouble(), coord[1].GetDouble()});
                    }
                }
            }

            return coordsCollector;
        }
    };
}