#pragma once

#include <string>

namespace TextureRenderingBuiltin {
    [[maybe_unused]]
    const std::string fragmentShader = R"(
#version 430 core

in vec2 p_uv;

out vec4 output_color;

uniform vec4 u_color;

void main() {
    output_color = vec4(u_color);
}
)";

    [[maybe_unused]]
    const std::string vertexShader = R"(
#version 430 core

layout(location = 0) in vec3 a_position;

uniform mat4 u_scale_matrix;
uniform mat4 u_translation_matrix;

void main() {
    mat4 v_combined_matrix = u_translation_matrix * u_scale_matrix;
    gl_Position = v_combined_matrix * vec4(a_position, 1.0f);
}
)";
}