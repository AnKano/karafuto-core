#pragma once

#include <glad/glad.h>

#include <string>
#include <utility>
#include <unordered_map>

namespace KCore::opengl {
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
        std::unordered_map<ShaderType, uint32_t> Shaders;
        const std::string Name;
        std::string Tag;

        uint32_t Id{0};

    public:
        Shader() : Shader("<some mShader>") {}

        explicit Shader(const std::string &name) : Id(glCreateProgram()), Name(name) {
            Tag = "KRen - mShader program [" + (name.empty() ? std::to_string(Id) : name) + "]";
        }

        void addShaderPairFromPath(const std::string &path) {
            std::string vsPath = path + "/vs.glsl";
            std::string fsPath = path + "/fs.glsl";

            uint32_t vsId = loadShaderFromFile(Vertex, vsPath);
            uint32_t fsId = loadShaderFromFile(Fragment, fsPath);

            std::string additional =
                    "| mShader id: " + std::to_string(vsId) + " and mShader id: " + std::to_string(fsId);
        }

        void addVertexShader(const std::string &source, ShaderSourceType sourceType = Text) {
            uint32_t id;

            switch (sourceType) {
                case Text:
                    id = loadShaderAsText(Vertex, source);
                    break;
                case File:
                    id = loadShaderFromFile(Vertex, source);
                    break;
                default:
                    //!TODO: throw more wise exception
                    throw std::exception();
            }

            std::string additional = "| mShader id: " + std::to_string(id);
        }

        void addFragmentShader(const std::string &source, ShaderSourceType sourceType = Text) {
            uint32_t id;

            switch (sourceType) {
                case Text:
                    id = loadShaderAsText(Fragment, source);
                    break;
                case File:
                    id = loadShaderFromFile(Fragment, source);
                    break;
                default:
                    //!TODO: throw more wise exception
                    throw std::exception();
            }

            std::string additional = "| mShader id: " + std::to_string(id);
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

        uint32_t loadShaderFromFile(const ShaderType &type, const std::string &path) {
            Shaders[type] = glCreateShader(type);
            auto content = load(path);
            compileShader(type, content);
            return Shaders[type];
        }

        uint32_t loadShaderAsText(const ShaderType &type, const std::string &text) {
            Shaders[type] = glCreateShader(type);
            compileShader(type, text);
            return Shaders[type];
        }

        void compileShader(const ShaderType &type, const std::string &source) {
            auto *c_string = source.c_str();

            glShaderSource(Shaders[type], 1, &c_string, nullptr);
            glCompileShader(Shaders[type]);
        }
    };
}