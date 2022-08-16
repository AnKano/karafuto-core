#pragma once

#include <vector>

#include "glm/vec3.hpp"

#include "GeoJSONObject.hpp"
#include "../../misc/Bindings.hpp"
#include "../../meshes/BaseMesh.hpp"

namespace KCore {
    struct GeoJSONTransObject {
        GeoJSONObjectType type;

        int mainShapeCoordsCount;
        int holeShapeCoordsCount;

        glm::vec3 *mainShapePositions;
        glm::vec3 *holeShapePositions;

        BaseMesh *mesh;

        float height;

        char* properties;
    };
}