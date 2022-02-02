#pragma once

#include <string>

namespace TextureRenderingBuiltin {
    [[maybe_unused]]
    const std::string fragmentShader = R"(
#version 430 core

in vec2 p_uv;

out vec4 output_color;

uniform vec4 u_color;
uniform sampler2D u_diffuse;

void main() {
    vec4 v_color = texture(u_diffuse, p_uv);
    output_color = vec4(v_color);
}
)";

    [[maybe_unused]]
    const std::string vertexShader = R"(
#version 430 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;

uniform mat4 u_scale_matrix;
uniform mat4 u_translation_matrix;

out vec2 p_uv;

void main() {
    p_uv = a_uv;

    mat4 v_combined_matrix = u_translation_matrix * u_scale_matrix;
    gl_Position = v_combined_matrix * vec4(a_position, 1.0f);
}
)";
}