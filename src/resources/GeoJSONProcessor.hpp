#pragma once

#include <vector>
#include <string>
#include <exception>
#include <iostream>

#include <rapidjson/document.h>

#include "../misc/NetworkTools.hpp"
#include "../misc/FileTools.hpp"

#include "../sources/local/geojson/primitives/GeoJSONTransObject.hpp"
#include "../sources/local/geojson/GeoJSONObject.hpp"

#include "meshes/PolylineMesh.hpp"
#include "meshes/PolygonMesh.hpp"

#include "../layer/LayerInterface.hpp"

namespace KCore {
    enum GeoJSONSourceType {
        Raw = 0,
        File = 1,
        Url = 2
    };

    std::vector<GeoJSONObject> ParseGeoJSON(const std::string &data);

    std::vector<GeoJSONTransObject> *ProcessGeoJSONObjects(Layer *layer, const std::vector<GeoJSONObject> &jsonObjects);

    std::vector<GeoJSONTransObject> *ProcessGeoJSONRaw(LayerInterface *layerPtr, const char *raw);

    std::vector<GeoJSONTransObject> *ProcessGeoJSONFile(LayerInterface *layerPtr, const char *path);

    std::vector<GeoJSONTransObject> *ProcessGeoJSONUrl(LayerInterface *layerPtr, const char *url);

    extern "C" {
    DllExport void ProcessGeoJSON(LayerInterface *layerPtr, GeoJSONSourceType type, const char *param);
    DllExport GeoJSONTransObject *EjectJSONObjectsFromVector(std::vector<GeoJSONTransObject> *vecPtr, int &length);
    }
}