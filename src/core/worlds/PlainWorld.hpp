#pragma once

#include "BaseWorld.hpp"

#include "../../bindings.hpp"
#include "../sources/BaseSource.hpp"
#include "../sources/local/srtm/SRTMLocalSource.hpp"
#include "../sources/local/geojson/GeoJSONLocalSource.hpp"

#include "stages/BuiltinStages.hpp"

namespace KCore {
    class PlainWorld : public BaseWorld {
    public:
        PlainWorld();

        PlainWorld(float latitude, float longitude);

        void makeEvents() override;

        void createTileResources(GenericTile *tile) override;

    private:
        void calculateTiles() override;
    };

    extern "C" {
    DllExport KCore::PlainWorld *CreatePlainWorld(float latitude, float longitude);
    }
}