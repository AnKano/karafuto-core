#include "Texture.hpp"
#include "VulkanCore.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(VulkanCore *parent, const std::vector<uint8_t> &data,
                 const uint32_t &width, const uint32_t &height, const uint32_t &bpp) : parent(parent) {
    createTextureImage(data, width, height, bpp);
    createTextureImageView(bpp);
    createTextureSampler();
}

void Texture::createTextureImage(const std::vector<uint8_t> &data,
                                 const uint32_t &width, const uint32_t &height, const uint32_t &bpp) {
    prepareTexture(data.data(), width, height, bpp);
}

void Texture::prepareTexture(const uint8_t *data, const uint32_t &width, const uint32_t &height, const uint32_t &bpp) {
    VkDeviceSize imageSize = width * height * bpp;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    parent->createBuffer(
            imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory
    );

    void *mem;
    vkMapMemory(parent->device, stagingBufferMemory, 0, imageSize, 0, &mem);
    memcpy(mem, data, static_cast<size_t>(imageSize));
    vkUnmapMemory(parent->device, stagingBufferMemory);

    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;

    parent->createImage(
            width, height, format, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            textureImage, textureImageMemory
    );

    parent->transitionImageLayout(textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    parent->copyBufferToImage(stagingBuffer, textureImage, width, height);
    parent->transitionImageLayout(textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(parent->device, stagingBuffer, nullptr);
    vkFreeMemory(parent->device, stagingBufferMemory, nullptr);
}

void Texture::createTextureSampler() {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(parent->physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(parent->device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void Texture::createTextureImageView(const uint32_t &bpp) {
    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    textureImageView = parent->createImageView(textureImage, format, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Texture::manuallyDestroy() {
    vkUnmapMemory(parent->device, textureImageMemory);
    vkFreeMemory(parent->device, textureImageMemory, nullptr);

    vkDestroySampler(parent->device, textureSampler, nullptr);
    vkDestroyImageView(parent->device, textureImageView, nullptr);

    vkDestroyImage(parent->device, textureImage, nullptr);
}

Texture::~Texture() {
//    vkUnmapMemory(parent->device, textureImageMemory);
    vkFreeMemory(parent->device, textureImageMemory, nullptr);

    vkDestroySampler(parent->device, textureSampler, nullptr);
    vkDestroyImageView(parent->device, textureImageView, nullptr);

    vkDestroyImage(parent->device, textureImage, nullptr);
}
