#pragma once

#include "../../bindings.hpp"

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace KCore {
    class BaseMesh {
    protected:
        std::vector<glm::vec3> mVertices;
        std::vector<glm::vec3> mNormals;
        std::vector<glm::vec2> mUVs;
        std::vector<uint32_t> mIndices;

    protected:
        virtual void createMesh();

    public:
        BaseMesh() = default;

        [[nodiscard]]
        const std::vector<glm::vec3> &getVertices() const;

        [[nodiscard]]
        const std::vector<glm::vec3> &getNormals() const;

        [[nodiscard]]
        const std::vector<glm::vec2> &getUVs() const;

        [[nodiscard]]
        const std::vector<uint32_t> &getIndices() const;

#ifdef __EMSCRIPTEN__
        intptr_t getIndices_ptr_emscripten() {
            return reinterpret_cast<intptr_t>(mIndices.data());
        }

        uint32_t getIndicesCount_emscripten() {
            return mIndices.size();
        }

        intptr_t getUVs_ptr_emscripten() {
            return reinterpret_cast<intptr_t>(mUVs.data());
        }

        uint32_t getUVsCount_emscripten() {
            return mUVs.size() * 2;
        }

        intptr_t getNormals_ptr_emscripten() {
            return reinterpret_cast<intptr_t>(mNormals.data());
        }

        uint32_t getNormalsCount_emscripten() {
            return mNormals.size() * 3;
        }

        intptr_t getVertices_ptr_emscripten() {
            return reinterpret_cast<intptr_t>(mVertices.data());
        }

        uint32_t getVerticesCount_emscripten() {
            return mVertices.size() * 3;
        }
#endif
    };

    extern "C" {
    DllExport float *GetMeshVertices(KCore::BaseMesh *mesh, int &length);

    DllExport float *GetMeshNormals(KCore::BaseMesh *mesh, int &length);

    DllExport float *GetMeshUVs(KCore::BaseMesh *mesh, int &length);

    DllExport unsigned int *GetMeshIndices(KCore::BaseMesh *mesh, int &length);
    }
}
