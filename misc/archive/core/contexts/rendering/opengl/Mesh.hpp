#pragma once

#include "Shader.hpp"

#include <GL/glew.h>

#include <optional>

namespace KCore::OpenGL {
    struct AttributeDescription {
        std::optional<std::string> AttributeName;
        std::optional<uint16_t> AttributeIdx;

        std::vector<uint8_t> AttributeData;
        uint32_t DataType;

        int32_t AttributeStrideLength;
        int32_t StrideElementLength;

        uint32_t AttributeVBO;
    };

    class MeshDescription {
    private:
        std::vector<OpenGL::AttributeDescription> DescribedAttribs;
        std::vector<uint32_t> Indices;

        KCore::OpenGL::Shader *Shader_ptr{};

    public:
        MeshDescription() = default;

        void setShader(KCore::OpenGL::Shader *shader) {
            Shader_ptr = shader;
        }

        void setIndicesBuffer(const std::vector<uint32_t> &indices) {
            Indices = indices;
        }

        void addAttribDescription(const OpenGL::AttributeDescription &data) {
            DescribedAttribs.push_back(data);
        }

        [[nodiscard]]
        KCore::OpenGL::Shader *getShader() {
            return Shader_ptr;
        };

        [[nodiscard]]
        std::vector<uint32_t> &get_indices() {
            return Indices;
        }

        [[nodiscard]]
        std::vector<OpenGL::AttributeDescription> &getDescribedAttribs() {
            return DescribedAttribs;
        }
    };

    class Mesh {
    private:
        uint32_t VAOId{0};
        std::unordered_map<uint16_t, AttributeDescription *> DescribedAttrib_ptrs{};

        uint32_t VBOIndices{0};
        uint32_t IndicesCount{0};

        Shader *Shader_ptr;

    public:
        Mesh(MeshDescription &mesh) {
            Shader_ptr = mesh.getShader();
            DescribedAttrib_ptrs = createVertexBuffers(mesh);

            VBOIndices = createIndexBuffer(mesh);
            IndicesCount = (uint32_t) mesh.get_indices().size();

            combineBuffers();
        }


        void draw() const {
            glBindVertexArray(VAOId);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOIndices);
            glDrawElements(GL_TRIANGLES, (int32_t) IndicesCount, GL_UNSIGNED_INT, nullptr);
        }

    private:

        void combineBuffers() {
            if (VAOId) return;

            uint32_t vaoId;
            glGenVertexArrays(1, &vaoId);
            glBindVertexArray(vaoId);

            for (const auto&[attribIdx, describedAttrib_ptr]: DescribedAttrib_ptrs) {
                glBindBuffer(GL_ARRAY_BUFFER, describedAttrib_ptr->AttributeVBO);
                glVertexAttribPointer(attribIdx,
                                      describedAttrib_ptr->AttributeStrideLength,
                                      describedAttrib_ptr->DataType,
                                      GL_FALSE,
                                      describedAttrib_ptr->AttributeStrideLength *
                                      describedAttrib_ptr->StrideElementLength,
                                      nullptr);
                glEnableVertexAttribArray(attribIdx);
            }

            VAOId = vaoId;
        }

        static std::unordered_map<uint16_t, AttributeDescription *> createVertexBuffers(MeshDescription &mesh) {
            std::unordered_map<uint16_t, AttributeDescription *> buffersIds{};

            auto &describedAttributes = mesh.getDescribedAttribs();
            for (auto &attribDesc: describedAttributes) {
                int32_t idx{-1};
                if (attribDesc.AttributeIdx != std::nullopt)
                    idx = attribDesc.AttributeIdx.value();
                else if (attribDesc.AttributeName != std::nullopt)
                    idx = glGetAttribLocation(mesh.getShader()->id(), attribDesc.AttributeName->c_str());
                if (idx < 0)
                    continue;

                attribDesc.AttributeIdx = idx;

                uint32_t bufferId;
                glGenBuffers(1, &bufferId);
                glBindBuffer(GL_ARRAY_BUFFER, bufferId);
                glBufferData(GL_ARRAY_BUFFER, attribDesc.AttributeData.size(),
                             attribDesc.AttributeData.data(), GL_STATIC_DRAW);

                std::string additional = "| buffer id: " + std::to_string(bufferId);

                // set declared index as key. key is attrib index
                attribDesc.AttributeVBO = bufferId;
                buffersIds[idx] = &attribDesc;
            }

            return buffersIds;
        }

        uint32_t createIndexBuffer(MeshDescription &mesh) {
            uint32_t bufferId;
            glGenBuffers(1, &bufferId);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, (int64_t) (mesh.get_indices().size() * sizeof(uint32_t)),
                         mesh.get_indices().data(), GL_STATIC_DRAW);

            std::string additional = "| buffer id: " + std::to_string(bufferId);

            return bufferId;
        }
    };
}
