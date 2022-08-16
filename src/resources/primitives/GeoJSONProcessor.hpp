#pragma once

#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "rapidjson/document.h"

#include "GeoJSONTransObject.hpp"
#include "GeoJSONObject.hpp"
#include "../elevation/IElevationSource.hpp"
#include "../../misc/NetworkTools.hpp"
#include "../../misc/FileTools.hpp"
#include "../../layer/LayerInterface.hpp"
#include "../../layer/Layer.hpp"

namespace KCore {
    enum GeoJSONSourceType {
        GeoJSONRaw = 0,
        GeoJSONFile = 1,
        GeoJSONUrl = 2
    };

    std::vector<GeoJSONObject> ParseGeoJSON(const std::string &data);

    std::vector<GeoJSONTransObject> *
    ProcessGeoJSONObjects(Layer *layer, const std::vector<GeoJSONObject> &jsonObjects,
                          IElevationSource *elevationSrc = nullptr);

    std::vector<GeoJSONTransObject> *
    ProcessGeoJSONRaw(LayerInterface *layerPtr, const char *raw, IElevationSource *elevationSrc = nullptr);

    std::vector<GeoJSONTransObject> *
    ProcessGeoJSONFile(LayerInterface *layerPtr, const char *path, IElevationSource *elevationSrc = nullptr);

    std::vector<GeoJSONTransObject> *
    ProcessGeoJSONUrl(LayerInterface *layerPtr, const char *url, IElevationSource *elevationSrc = nullptr);

    extern "C" {
    DllExport std::vector<GeoJSONTransObject> *
    ProcessGeoJSON(LayerInterface *layerPtr, GeoJSONSourceType type, const char *param);

    DllExport std::vector<GeoJSONTransObject> *
    ProcessGeoJSONWithElevation(LayerInterface *layerPtr, IElevationSource *elevation, GeoJSONSourceType type,
                                const char *param);

    DllExport GeoJSONTransObject *EjectJSONObjectsFromVector(std::vector<GeoJSONTransObject> *vecPtr, int &length);
    }
}