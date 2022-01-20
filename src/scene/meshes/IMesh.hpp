#pragma once

#include "glm/glm.hpp"
#include <vector>

namespace KCore {
    class IMesh {
    protected:
        std::vector<glm::vec3> Vertices;
        std::vector<glm::vec3> Normals;
        std::vector<glm::vec2> UVs;
        std::vector<uint32_t> Indices;

    public:
        IMesh() = default;

        virtual void createMesh() = 0;

        intptr_t getIndices_ptr_emscripten() {
            return reinterpret_cast<intptr_t>(Indices.data());
        }

        uint32_t getIndicesCount_emscripten() {
            return Indices.size();
        }

        intptr_t getUVs_ptr_emscripten() {
            return reinterpret_cast<intptr_t>(UVs.data());
        }

        uint32_t getUVsCount_emscripten() {
            return UVs.size() * 2;
        }

        intptr_t getNormals_ptr_emscripten() {
            return reinterpret_cast<intptr_t>(Normals.data());
        }

        uint32_t getNormalsCount_emscripten() {
            return Normals.size() * 3;
        }

        intptr_t getVertices_ptr_emscripten() {
            return reinterpret_cast<intptr_t>(Vertices.data());
        }

        uint32_t getVerticesCount_emscripten() {
            return Vertices.size() * 3;
        }

        [[nodiscard]]
        const std::vector<glm::vec3> &getVertices() const {
            return Vertices;
        }

        [[nodiscard]]
        const std::vector<glm::vec3> &getNormals() const {
            return Normals;
        }

        [[nodiscard]]
        const std::vector<glm::vec2> &getUVs() const {
            return UVs;
        }

        [[nodiscard]]
        const std::vector<uint32_t> &getIndices() const {
            return Indices;
        }
    };
}
