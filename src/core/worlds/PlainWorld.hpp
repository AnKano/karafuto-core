#pragma once

#include "BaseWorld.hpp"

#include "../../bindings.hpp"
#include "../sources/BaseSource.hpp"
#include "../sources/local/srtm/SRTMLocalSource.hpp"
#include "../sources/local/geojson/GeoJSONLocalSource.hpp"

namespace KCore {
    class PlainWorld : public BaseWorld {
    private:
        BaseSource *srtmPrimitives;
        BaseSource *jsonPrimitives;

    public:
        PlainWorld() : BaseWorld(0.0f, 0.0f) {}

        PlainWorld(float latitude, float longitude) : BaseWorld(latitude, longitude) {
            srtmPrimitives = new KCore::SRTMLocalSource;
            srtmPrimitives->addSourcePart("assets/sources", ".hgt");

            jsonPrimitives = new KCore::GeoJSONLocalSource;
            jsonPrimitives->addSourcePart("assets/sources/points.geojson");
        }

        void makeEvents() override;

    private:
        void calculateTiles() override;
    };

    extern "C" {
    DllExport KCore::PlainWorld *CreatePlainWorld(float latitude, float longitude);
    }
}