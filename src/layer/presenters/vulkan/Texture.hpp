#pragma once

#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>

class VulkanCore;

class Texture {
public:
    VulkanCore *parent;

    VkImage textureImage;
    VkImageView textureImageView;
    VkDeviceMemory textureImageMemory;
    VkSampler textureSampler;

    Texture(VulkanCore *parent, const std::string &path);

    Texture(VulkanCore *parent, const std::vector<uint8_t> &data,
            const uint32_t &width, const uint32_t &height, const uint32_t &bpp);

    ~Texture();

    void manuallyDestroy();

private:
    void createTextureImage(const std::string &path);

    void createTextureImage(const std::vector<uint8_t> &data,
                            const uint32_t &width, const uint32_t &height, const uint32_t &bpp);

    void createTextureSampler();

    void createTextureImageView();

    void prepareTexture(const uint8_t *data, const uint32_t &width, const uint32_t &height, const uint32_t &bpp);
};
