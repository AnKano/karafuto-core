#pragma once

#include <array>
#include <exception>
#include <map>
#include <string>
#include <vector>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace KCore {
    enum GeoJSONObjectType {
        Point = 0,
        Polyline = 1,
        Polygon = 2,
        PolygonWithHole = 3
    };

    struct GeoJSONObject {
    public:
        GeoJSONObjectType mType;

        std::vector<std::array<double, 2>> mMainShapeCoords; // main shape coords
        std::vector<std::array<double, 2>> mHoleShapeCoords; // hole shape coords

        std::string mPropertiesObject;

    public:
        explicit GeoJSONObject(const rapidjson::Value &value) {
            restoreFromString(value);
        }

    private:
        void restoreFromString(const rapidjson::Value &value) {
            for (const auto &item: std::vector{"geometry", "type", "properties"}) {
                if (!value.HasMember(item))
                    throw std::runtime_error("undefined file structure!");
            }

            auto &geometry = value["geometry"];
            if (!geometry.IsObject()) throw std::runtime_error("undefined file structure!");
            for (const auto &item: std::vector{"coordinates", "type"}) {
                if (!geometry.HasMember(item))
                    throw std::runtime_error("undefined file structure!");
            }

            // get type of geometry
            mType = getType(geometry["type"]);

            // get coords
            auto mCoords = getCoords(geometry["coordinates"]);

            mMainShapeCoords = mCoords[0];
            mHoleShapeCoords = mCoords[1];

            if (value.HasMember("properties")) {
                rapidjson::StringBuffer sb;
                sb.Clear();

                rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
                value["properties"].Accept(writer);

                mPropertiesObject = sb.GetString();
            }
        }

        static GeoJSONObjectType getType(const rapidjson::Value &value) {
            if (!value.IsString()) throw std::runtime_error("undefined file structure!");

            auto str = std::string{value.GetString()};
            if (str == "Point") return Point;
            if (str == "LineString") return Polyline;
            if (str == "Polygon") return Polygon;

            throw std::runtime_error("undefined type of feature!");
        }

        std::array<std::vector<std::array<double, 2>>, 2> getCoords(const rapidjson::Value &value) {
            std::array<std::vector<std::array<double, 2>>, 2> coordsCollector;

            if (!value.IsArray()) throw std::runtime_error("undefined file structure!");

            // parse as point
            if (mType == Point) {
                if (value.Size() != 2) throw std::runtime_error("undefined file structure!");
                coordsCollector[0].push_back({value[0].GetDouble(), value[1].GetDouble()});
            }

            if (mType == Polyline) {
                for (const auto &nestedValue: value.GetArray()) {
                    if (nestedValue.Size() != 2) throw std::runtime_error("undefined file structure!");
                    coordsCollector[0].push_back({nestedValue[0].GetDouble(), nestedValue[1].GetDouble()});
                }
            }

            if (mType == Polygon) {
                auto shapesCount = value.Size();
                if (!value.IsArray() || shapesCount > 2) throw std::runtime_error("undefined file structure!");

                if (shapesCount == 2) mType = PolygonWithHole;

                for (std::size_t shape = 0; shape < shapesCount; shape++) {
                    const auto &shapeCoords = value[shape].GetArray();
                    for (const auto &coord: shapeCoords) {
                        if (coord.Size() != 2) throw std::runtime_error("undefined file structure!");
                        coordsCollector[shape].push_back({coord[0].GetDouble(), coord[1].GetDouble()});
                    }
                }
            }

            return coordsCollector;
        }
    };
}