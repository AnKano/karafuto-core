#pragma once

#include <exception>

#include "../../FileSource.hpp"
#include "GeoJSONObject.hpp"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>

namespace KCore {
    class GeoJSONFileSourcePiece : public FileSource {
    private:
        std::vector<GeoJSONObject> mObjects;
        std::string mDataString;

    public:
        GeoJSONFileSourcePiece(const std::string &path) : FileSource(path) {
            try {
                parse();
            } catch (const std::exception &ex) {
                std::cerr << ex.what() << std::endl;
            }
        }

        const std::vector<GeoJSONObject> &getObjects() {
            return mObjects;
        }

    private:
        void parse() {

            rapidjson::Document doc;

            mDataString = std::string{mData.data(), mData.data() + mData.size()};
            if (doc.Parse(mDataString.c_str()).HasParseError()) {
                throw std::runtime_error(
                        "Error occurred in parsing: " + std::to_string(doc.GetParseError())
                );
            }

            auto type = std::string{doc["type"].GetString()};
            if (type == "FeatureCollection") {
                parseCollection(doc["features"]);
            } else if (type == "Feature") {
                processObject(doc);
            } else {
                mFileCorrupted = true;
            }


        }

        void parseCollection(const rapidjson::Value &collection) {
            for (const auto &val: collection.GetArray()) {
                try {
                    processObject(val);
                } catch (const std::exception &ex) {
                    std::cerr << ex.what() << std::endl;
                }
            }
        }

        void processObject(const rapidjson::Value &object) {
            mObjects.emplace_back(object);
        }
    };
}