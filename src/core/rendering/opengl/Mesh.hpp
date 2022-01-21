#pragma once

#include "glad/glad.h"

#include "Shader.hpp"

namespace KCore::opengl {
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
        std::vector<opengl::AttributeDescription> DescribedAttribs;
        std::vector<uint32_t> Indices;

        KCore::opengl::Shader *Shader_ptr{};

    public:
        MeshDescription() = default;

        void setShader(KCore::opengl::Shader *shader) {
            Shader_ptr = shader;
        }

        void setIndicesBuffer(const std::vector<uint32_t> &indices) {
            Indices = indices;
        }

        void addAttribDescription(const opengl::AttributeDescription &data) {
            DescribedAttribs.push_back(data);
        }

        [[nodiscard]]
        KCore::opengl::Shader *getShader() {
            return Shader_ptr;
        };

        [[nodiscard]]
        std::vector<uint32_t> &get_indices() {
            return Indices;
        }

        [[nodiscard]]
        std::vector<opengl::AttributeDescription> &getDescribedAttribs() {
            return DescribedAttribs;
        }
    };

    class Mesh {
    private:
        std::string Tag{"KRen mMesh"};

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

            //kren::engine::log(Tag, "Initial mMesh buffers successfully declared", info);

            combineBuffers();

            //kren::engine::log(Tag, "Initial mMesh buffers successfully instantiated", info);
        }

        ~Mesh() {
            for (const auto&[attribId, describedAttrib_ptr]: DescribedAttrib_ptrs)
                glDeleteBuffers(1, &describedAttrib_ptr->AttributeVBO);
            glDeleteBuffers(1, &VBOIndices);
        }

        void draw() const {
            glBindVertexArray(VAOId);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOIndices);
            glDrawElements(GL_TRIANGLES, (int32_t) IndicesCount, GL_UNSIGNED_INT, nullptr);
        }

    private:
        void combineBuffers() {
            if (VAOId) {
                //kren::engine::log(Tag, "Attempt to declare already declared mMesh!", info);
                return;
            }

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
            //kren::engine::log(Tag, "Successfully declared ", info);
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
                if (idx < 0) {
                    //kren::engine::log(Tag, "Skip wrongly described attribute", fail);
                    continue;
                }

                attribDesc.AttributeIdx = idx;

                uint32_t bufferId;
                glGenBuffers(1, &bufferId);
                glBindBuffer(GL_ARRAY_BUFFER, bufferId);
                glBufferData(GL_ARRAY_BUFFER, attribDesc.AttributeData.size(),
                             attribDesc.AttributeData.data(), GL_STATIC_DRAW);

                std::string additional = "| buffer id: " + std::to_string(bufferId);
                //kren::engine::log(Tag, "mVertices buffer declared " + additional, info);

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
            //kren::engine::log(Tag, "mIndices buffer declared " + additional, info);

            return bufferId;
        }
    };
}
