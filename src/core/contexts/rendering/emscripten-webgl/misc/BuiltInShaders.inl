#pragma once

#include <string>

namespace KCore::WebGL::BuiltIn::TextureRenderer {
    static const std::string vertexShaderSource = {
            "#version 300 es\n"
            "in vec3 a_position;\n"
            "in vec2 a_uv;\n"
            "uniform mat4 u_scale_matrix;\n"
            "uniform mat4 u_translation_matrix;\n"
            "out vec2 p_uv;\n"
            "void main() {\n"
            "  p_uv = a_uv;\n"
            "  mat4 v_combined_matrix = u_translation_matrix * u_scale_matrix;\n"
            "  gl_Position = v_combined_matrix * vec4(a_position, 1.0f);\n"
            "}\n"
    };

    static const std::string fragmentShaderSource = {
            "#version 300 es\n"
            "precision mediump float;\n"
            "in vec2 p_uv;\n"
            "uniform vec4 u_color;\n"
            "uniform sampler2D u_diffuse;\n"
            "out vec4 fragColor;\n"
            "void main() {\n"
            "  vec4 v_color = texture(u_diffuse, p_uv);\n"
            "  fragColor = v_color;\n"
            "}\n"
    };

//    static const std::string vertexShaderSource = {
//            "#version 300 es\n"
//            "in vec4 position;\n"
//            "void main() {\n"
//            "  gl_Position = vec4(position.xyz, 1.0);\n"
//            "}\n"
//    };
//
//    static const std::string fragmentShaderSource = {
//            "#version 300 es\n"
//            "precision mediump float;\n"
//            "out vec4 fragColor;\n"
//            "void main() {\n"
//            "  fragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
//            "}\n"
//    };

//    [[maybe_unused]]
//    const std::string fs = R"(
//#version 300 es
//
//in vec2 p_uv;
//
//out vec4 output_color;
//
//uniform vec4 u_color;
//uniform sampler2D u_diffuse;
//
//void main() {
//    vec4 v_color = texture(u_diffuse, p_uv);
//    output_color = vec4(v_color);
//}
//)";
//
//    [[maybe_unused]]
//    const std::string vs = R"(
//#version 300 es
//
//in vec3 a_position;
//in vec2 a_uv;
//
//uniform mat4 u_scale_matrix;
//uniform mat4 u_translation_matrix;
//
//out vec2 p_uv;
//
//void main() {
//    p_uv = a_uv;
//
//    mat4 v_combined_matrix = u_translation_matrix * u_scale_matrix;
//    gl_Position = v_combined_matrix * vec4(a_position, 1.0f);
//}
//)";
}