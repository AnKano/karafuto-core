#pragma once

#include <glad/glad.h>

#include <string>
#include <utility>
#include <unordered_map>

namespace KCore::OpenGL {
    class Shader {
    public:
        enum ShaderType {
            Vertex = GL_VERTEX_SHADER,
            Fragment = GL_FRAGMENT_SHADER
        };

        enum ShaderSourceType {
            Text, File
        };

    private:
        uint32_t Id{0};

        std::unordered_map<ShaderType, uint32_t> Shaders;

    public:
        explicit Shader() : Id(glCreateProgram()) {}

        void addShaderPairFromPath(const std::string &path) {
            std::string vsPath = path + "/vs.glsl";
            std::string fsPath = path + "/fs.glsl";

            loadShaderFromFile(Vertex, vsPath);
            loadShaderFromFile(Fragment, fsPath);
        }

        void addVertexShader(const std::string &source, ShaderSourceType sourceType = Text) {
            switch (sourceType) {
                case Text:
                    loadShaderAsText(Vertex, source);
                    break;
                case File:
                    loadShaderFromFile(Vertex, source);
                    break;
                default:
                    //!TODO: throw more wise exception
                    throw std::exception();
            }
        }

        void addFragmentShader(const std::string &source, ShaderSourceType sourceType = Text) {
            switch (sourceType) {
                case Text:
                    loadShaderAsText(Fragment, source);
                    break;
                case File:
                    loadShaderFromFile(Fragment, source);
                    break;
                default:
                    //!TODO: throw more wise exception
                    throw std::exception();
            }
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
        static std::string load(const std::string &filename) {
            auto *file = fopen(filename.c_str(), "r");
            //!TODO: throw more wise exception
            if (!file) throw std::exception();

            std::string src;
            int c;
            while ((c = getc(file)) != EOF)
                src.push_back((char) c);

            return src;
        }

        void loadShaderFromFile(const ShaderType &type, const std::string &path) {
            Shaders[type] = glCreateShader(type);
            auto content = load(path);
            compileShader(type, content);
        }

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