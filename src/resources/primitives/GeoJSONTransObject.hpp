#pragma once

#include "../../misc/Bindings.hpp"
#include "meshes/BaseMesh.hpp"
#include "GeoJSONObject.hpp"

#include "glm/vec3.hpp"

#include <vector>

namespace KCore {
    struct GeoJSONTransObject {
        GeoJSONObjectType type;

        int mainShapeCoordsCount;
        int holeShapeCoordsCount;

        glm::vec3 *mainShapePositions;
        glm::vec3 *holeShapePositions;

        BaseMesh *mesh;

        float height;
    };
}