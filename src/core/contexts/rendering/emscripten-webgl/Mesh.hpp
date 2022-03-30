#pragma once

#include "Shader.hpp"

#include <GL/glew.h>

#include <optional>
#include <vector>

namespace KCore::WebGL {
    struct AttributeDescription {
        std::optional<std::string> mAttributeName;
        std::optional<uint16_t> mAttributeIdx;

        std::vector<uint8_t> mAttributeData;
        uint32_t mDataType;

        int32_t mAttributeStrideLength;
        int32_t mStrideElementLength;

        uint32_t mAttributeVBO;
    };

    class MeshDescription {
    private:
        std::vector<WebGL::AttributeDescription> mDescribedAttribs;
        std::vector<uint32_t> mIndices;

        KCore::WebGL::Shader *mShaderPtr{};

    public:
        MeshDescription() = default;

        void setShader(KCore::WebGL::Shader *shader) {
            mShaderPtr = shader;
        }

        void setIndicesBuffer(const std::vector<uint32_t> &indices) {
            mIndices = indices;
        }

        void addAttribDescription(const WebGL::AttributeDescription &data) {
            mDescribedAttribs.push_back(data);
        }

        [[nodiscard]]
        KCore::WebGL::Shader *getShader() {
            return mShaderPtr;
        };

        [[nodiscard]]
        std::vector<uint32_t> &get_indices() {
            return mIndices;
        }

        [[nodiscard]]
        std::vector<WebGL::AttributeDescription> &getDescribedAttribs() {
            return mDescribedAttribs;
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
                glBindBuffer(GL_ARRAY_BUFFER, describedAttrib_ptr->mAttributeVBO);
                glVertexAttribPointer(attribIdx,
                                      describedAttrib_ptr->mAttributeStrideLength,
                                      describedAttrib_ptr->mDataType,
                                      GL_FALSE,
                                      describedAttrib_ptr->mAttributeStrideLength *
                                      describedAttrib_ptr->mStrideElementLength,
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
                if (attribDesc.mAttributeIdx != std::nullopt)
                    idx = attribDesc.mAttributeIdx.value();
                else if (attribDesc.mAttributeName != std::nullopt)
                    idx = glGetAttribLocation(mesh.getShader()->id(), attribDesc.mAttributeName->c_str());
                if (idx < 0)
                    continue;

                attribDesc.mAttributeIdx = idx;

                uint32_t bufferId;
                glGenBuffers(1, &bufferId);
                glBindBuffer(GL_ARRAY_BUFFER, bufferId);
                glBufferData(GL_ARRAY_BUFFER, attribDesc.mAttributeData.size(),
                             attribDesc.mAttributeData.data(), GL_STATIC_DRAW);

//                std::string additional = "| buffer id: " + std::to_string(bufferId);

                // set declared index as key. key is attrib index
                attribDesc.mAttributeVBO = bufferId;
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

//            std::string additional = "| buffer id: " + std::to_string(bufferId);

            return bufferId;
        }
    };
}
