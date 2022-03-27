#include "BaseMesh.hpp"

namespace KCore {
    void BaseMesh::createMesh() {}

    const std::vector<glm::vec3> &BaseMesh::getVertices() const {
        return mVertices;
    }

    const std::vector<glm::vec3> &BaseMesh::getNormals() const {
        return mNormals;
    }

    const std::vector<glm::vec2> &BaseMesh::getUVs() const {
        return mUVs;
    }

    const std::vector<uint32_t> &BaseMesh::getIndices() const {
        return mIndices;
    }

#ifndef EMSCRIPTEN
    DllExport float *GetMeshVertices(KCore::BaseMesh *mesh, int &length) {
        auto &vertices = mesh->getVertices();
        length = 3 * (int) vertices.size();
        return const_cast<float *>(glm::value_ptr(vertices[0]));
    }

    DllExport float *GetMeshNormals(KCore::BaseMesh *mesh, int &length) {
        auto &normals = mesh->getNormals();
        length = 3 * (int) normals.size();
        return const_cast<float *>(glm::value_ptr(normals[0]));
    }

    DllExport float *GetMeshUVs(KCore::BaseMesh *mesh, int &length) {
        auto &uvs = mesh->getUVs();
        length = 2 * (int) uvs.size();
        return const_cast<float *>(glm::value_ptr(uvs[0]));
    }

    DllExport unsigned int *GetMeshIndices(KCore::BaseMesh *mesh, int &length) {
        auto &indices = mesh->getIndices();
        length = (int) indices.size();
        return const_cast<unsigned int *>(indices.data());
    }
#endif
}