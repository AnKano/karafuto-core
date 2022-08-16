#pragma once

#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "../misc/Bindings.hpp"

namespace KCore {
    class BaseMesh {
    protected:
        std::vector<glm::vec3> mPositions;
        std::vector<glm::vec3> mNormals;
        std::vector<glm::vec2> mUVs;
        std::vector<uint32_t> mIndices;

    protected:
        virtual void createMesh();

    public:
        BaseMesh() = default;

        [[nodiscard]] const std::vector<glm::vec3> &getVertices() const;

        [[nodiscard]] const std::vector<glm::vec3> &getNormals() const;

        [[nodiscard]] const std::vector<glm::vec2> &getUVs() const;

        [[nodiscard]] const std::vector<uint32_t> &getIndices() const;
    };

    extern "C" {
    DllExport float *GetMeshVertices(KCore::BaseMesh *mesh, int &length);

    DllExport float *GetMeshNormals(KCore::BaseMesh *mesh, int &length);

    DllExport float *GetMeshUVs(KCore::BaseMesh *mesh, int &length);

    DllExport unsigned int *GetMeshIndices(KCore::BaseMesh *mesh, int &length);
    }
}
