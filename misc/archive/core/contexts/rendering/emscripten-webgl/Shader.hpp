#pragma once

#include <GL/glew.h>

#include <string>
#include <utility>
#include <unordered_map>

namespace KCore::WebGL {
    class Shader {
    public:
        enum ShaderType {
            Vertex = GL_VERTEX_SHADER,
            Fragment = GL_FRAGMENT_SHADER
        };

    private:
        uint32_t Id{0};

        std::unordered_map<ShaderType, uint32_t> Shaders;

    public:
        explicit Shader() : Id(glCreateProgram()) {}

        void addVertexShader(const std::string &source) {
            loadShaderAsText(Vertex, source);
        }

        void addFragmentShader(const std::string &source) {
            loadShaderAsText(Fragment, source);
        }

        void build() {
            for (auto const&[_, shader]: Shaders)
                glAttachShader(Id, shader);
            glLinkProgram(Id);
        };

        [[nodiscard]]
        int32_t uniform_position(const std::string &name) const {
            return glGetUniformLocation(Id, name.c_str());
        }

        [[nodiscard]]
        uint32_t id() const {
            return Id;
        }

        void bind() const {
            glUseProgram(Id);
        }

    private:
        void loadShaderAsText(const ShaderType &type, const std::string &text) {
            Shaders[type] = glCreateShader(type);
            compileShader(type, text);
        }

        void compileShader(const ShaderType &type, const std::string &source) {
            auto *c_string = source.c_str();

            glShaderSource(Shaders[type], 1, &c_string, nullptr);
            glCompileShader(Shaders[type]);
        }
    };
}