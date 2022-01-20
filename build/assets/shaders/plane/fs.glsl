#version 430 core

in vec2 p_uv;

out vec4 output_color;

uniform vec4 u_color;

void main() {
    output_color = vec4(u_color);
}