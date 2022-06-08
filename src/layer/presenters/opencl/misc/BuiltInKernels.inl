#pragma once

#include <string>

namespace KCore::OpenCL::BuiltIn::TextureRenderer {
        [[maybe_unused]]
        const std::string kernel = R"(
__constant sampler_t config = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_REPEAT | CLK_FILTER_LINEAR;

__kernel void oneShotRenderToRGB888(
        __read_only image2d_t input,
        __global char* output,

        unsigned int offset_width,
        unsigned int offset_height,

        unsigned int target_width,
        unsigned int target_height
) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    float normTexX = (float) pos.x / (float) target_width;
    float normTexY = (float) pos.y / (float) target_height;
    uint4 texelData = read_imageui(input, config, (float2)(normTexX, normTexY));

    int pos_x = offset_width + pos.x;
    int pos_y = offset_height + pos.y;

    int pixel = pos_y * 1024 + pos_x;

    output[pixel*3+0] = texelData.x;
    output[pixel*3+1] = texelData.y;
    output[pixel*3+2] = texelData.z;
}

__kernel void oneShotRenderToRGB565(
        __read_only image2d_t input,
        __global char* output,

        unsigned int offset_width,
        unsigned int offset_height,

        unsigned int target_width,
        unsigned int target_height
) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    float normTexX = (float) pos.x / (float) target_width;
    float normTexY = (float) pos.y / (float) target_height;
    uint4 texelData = read_imageui(input, config, (float2)(normTexX, normTexY));

    int pos_x = offset_width + pos.x;
    int pos_y = offset_height + pos.y;

    int pixel = pos_y * 1024 + pos_x;

    char x1, x2;
    x1 = (texelData.x & 0xF8) | (texelData.y >> 5);
    x2 = ((texelData.y & 0x1C) << 3) | (texelData.z  >> 3);

    output[pixel*2+0] = x2;
    output[pixel*2+1] = x1;
}

__kernel void wipeCanvas(__global char* output) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    int pixel = pos.y * 1024 + pos.x;

    output[pixel*2+0] = 0x00;
    output[pixel*2+1] = 0x00;
}
)";
    }