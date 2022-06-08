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
                } catch (...) {

                }
            }
        } else if (type == "Feature")
            try {
                accumulator.emplace_back(doc);
            } catch (...) {

            }
        else
            throw std::runtime_error("file corrupted");

        return accumulator;
    }

    std::vector<GeoJSONTransObject> *
    ProcessGeoJSONObjects(Layer *layer, const std::vector<GeoJSONObject> &jsonObjects) {
        auto *objects = new std::vector<KCore::GeoJSONTransObject>();

        for (auto &geojsonObject: jsonObjects) {
            KCore::GeoJSONTransObject obj{
                    geojsonObject.mType,
                    (int) (geojsonObject.mMainShapeCoords.size()),
                    (int) (geojsonObject.mHoleShapeCoords.size()),
                    nullptr, nullptr
            };
            auto convertedMain = std::vector<std::array<double, 2>>{};
            auto convertedHole = std::vector<std::array<double, 2>>{};

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

    std::vector<GeoJSONTransObject> *ProcessGeoJSONRaw(LayerInterface *layerPtr, const char *raw) {
        return ProcessGeoJSONObjects(layerPtr->raw(), ParseGeoJSON(raw));
    }

    std::vector<GeoJSONTransObject> *ProcessGeoJSONFile(LayerInterface *layerPtr, const char *path) {
        auto download = fileRead(path);
        auto str = std::string{download.begin(), download.end()};
        return ProcessGeoJSONObjects(layerPtr->raw(), ParseGeoJSON(str));
    }

    std::vector<GeoJSONTransObject> *ProcessGeoJSONUrl(LayerInterface *layerPtr, const char *url) {
        auto download = performGETRequestSync(url);
        auto str = std::string{download.begin(), download.end()};
        return ProcessGeoJSONObjects(layerPtr->raw(), ParseGeoJSON(str));
    }

    DllExport GeoJSONTransObject *EjectJSONObjectsFromVector(std::vector<GeoJSONTransObject> *vecPtr, int &length) {
        length = (int) vecPtr->size();
        return vecPtr->data();
    }

    DllExport void ProcessGeoJSON(LayerInterface *layerPtr, GeoJSONSourceType type, const char *param) {
        switch (type) {
            case Raw:
                ProcessGeoJSONRaw(layerPtr, param);
                break;
            case File:
                ProcessGeoJSONFile(layerPtr, param);
                break;
            case Url:
                ProcessGeoJSONUrl(layerPtr, param);
                break;
        }
    }
}