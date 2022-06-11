#include "GeoJSONProcessor.hpp"


namespace KCore {
    std::vector<GeoJSONObject> ParseGeoJSON(const std::string &data) {
        std::vector<GeoJSONObject> accumulator;

        rapidjson::Document doc;

        if (doc.Parse(data.c_str()).HasParseError())
            throw std::runtime_error("error occurred in parsing: " + std::to_string(doc.GetParseError()));

        auto type = std::string{doc["type"].GetString()};

        if (type == "FeatureCollection") {
            for (const auto &feature: doc["features"].GetArray()) {
                try {
                    accumulator.emplace_back(feature);
                } catch (const std::exception& ex) {
                    std::cerr << ex.what() << std::endl;
                }
            }
        } else if (type == "Feature")
            try {
                accumulator.emplace_back(doc);
            } catch (const std::exception& ex) {
                std::cerr << ex.what() << std::endl;
            }
        else throw std::runtime_error("unsupported file format!");

        return accumulator;
    }

    std::vector<GeoJSONTransObject> *
    ProcessGeoJSONObjects(Layer *layer, const std::vector<GeoJSONObject> &jsonObjects, IElevationSrc* elevationSrc) {
        auto *objects = new std::vector<KCore::GeoJSONTransObject>();

        for (auto &geojsonObject: jsonObjects) {
            KCore::GeoJSONTransObject obj{
                    geojsonObject.mType,
                    (int) (geojsonObject.mMainShapeCoords.size()),
                    (int) (geojsonObject.mHoleShapeCoords.size()),
                    nullptr, nullptr, nullptr, 0.0f, nullptr
            };
            auto convertedMain = std::vector<std::array<double, 2>>{};
            auto convertedHole = std::vector<std::array<double, 2>>{};

            if (!geojsonObject.mPropertiesObject.empty()) {
                auto len = geojsonObject.mPropertiesObject.size();
                auto str = geojsonObject.mPropertiesObject;

                obj.properties = new char[len + 1];
                std::strcpy(obj.properties, str.c_str());
            }

            if (elevationSrc != nullptr) {
                auto height = FLT_MAX;
                for (const auto &item: geojsonObject.mMainShapeCoords) {
                    auto res = elevationSrc->getElevationAtLatLon(item[0], item[1]);
                    if (res < height) height = res;
                }
                obj.height = height;
            }

            if (obj.mainShapeCoordsCount > 0) {
                obj.mainShapePositions = new glm::vec3[obj.mainShapeCoordsCount];
                for (int idx = 0; idx < obj.mainShapeCoordsCount; idx++) {
                    auto project = layer->latLonToWorldPosition(
                            {geojsonObject.mMainShapeCoords[idx][1], geojsonObject.mMainShapeCoords[idx][0]}
                    );
                    convertedMain.push_back({project.x, project.y});
                    obj.mainShapePositions[idx] = {project.x, 0.0f, project.y};
                }
            }

            if (obj.holeShapeCoordsCount > 0) {
                auto coordsCount = obj.holeShapeCoordsCount;
                obj.holeShapePositions = new glm::vec3[coordsCount];
                for (int idx = 0; idx < coordsCount; idx++) {
                    auto project = layer->latLonToWorldPosition(
                            {geojsonObject.mHoleShapeCoords[idx][1], geojsonObject.mHoleShapeCoords[idx][0]}
                    );
                    convertedHole.push_back({project.x, project.y});
                    obj.holeShapePositions[idx] = {project.x, 0.0f, project.y};
                }
            }

            if (geojsonObject.mType == KCore::Polyline)
                obj.mesh = new KCore::PolylineMesh(geojsonObject, convertedMain);

            if (geojsonObject.mType == KCore::Polygon || geojsonObject.mType == KCore::PolygonWithHole)
                obj.mesh = new KCore::PolygonMesh(geojsonObject, convertedMain, convertedHole);

            objects->push_back(obj);
        }

        return objects;
    }

    std::vector<GeoJSONTransObject> *
    ProcessGeoJSONRaw(LayerInterface *layerPtr, const char *raw, IElevationSrc *elevationSrc) {
        return ProcessGeoJSONObjects(layerPtr->raw(), ParseGeoJSON(raw), elevationSrc);
    }

    std::vector<GeoJSONTransObject> *
    ProcessGeoJSONFile(LayerInterface *layerPtr, const char *path, IElevationSrc *elevationSrc) {
        auto download = readFile(path);
        auto str = std::string{download.begin(), download.end()};
        return ProcessGeoJSONObjects(layerPtr->raw(), ParseGeoJSON(str), elevationSrc);
    }

    std::vector<GeoJSONTransObject> *
    ProcessGeoJSONUrl(LayerInterface *layerPtr, const char *url, IElevationSrc *elevationSrc) {
        auto download = performGETRequestSync(url);
        auto str = std::string{download.begin(), download.end()};
        return ProcessGeoJSONObjects(layerPtr->raw(), ParseGeoJSON(str), elevationSrc);
    }

    DllExport GeoJSONTransObject *EjectJSONObjectsFromVector(std::vector<GeoJSONTransObject> *vecPtr, int &length) {
        length = (int) vecPtr->size();
        return vecPtr->data();
    }

    DllExport std::vector<GeoJSONTransObject> *
    ProcessGeoJSONWithElevation(LayerInterface *layerPtr, IElevationSrc *elevation, GeoJSONSourceType type,
                                const char *param) {
        switch (type) {
            case GeoJSONRaw:
                return ProcessGeoJSONRaw(layerPtr, param, elevation);
            case GeoJSONFile:
                return ProcessGeoJSONFile(layerPtr, param, elevation);
            case GeoJSONUrl:
                return ProcessGeoJSONUrl(layerPtr, param, elevation);
        }

        return nullptr;
    }

    DllExport std::vector<GeoJSONTransObject> *
    ProcessGeoJSON(LayerInterface *layerPtr, GeoJSONSourceType type, const char *param) {
        switch (type) {
            case GeoJSONRaw:
                return ProcessGeoJSONRaw(layerPtr, param);
            case GeoJSONFile:
                return ProcessGeoJSONFile(layerPtr, param);
            case GeoJSONUrl:
                return ProcessGeoJSONUrl(layerPtr, param);
        }

        return nullptr;
    }
}