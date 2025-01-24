#include "texture_manager.h"

#include <engine.h>
#include <renderer_vk_images.h>
#include <renderer_vk_types.h>
#include <stdexcept>
#include <utility.h>
#include <vendor/stb/stb_image.h>

TextureManager* TextureManager::mInstance = nullptr;

TextureManager::TextureManager(Engine &engine)
	: mEngine(engine)
{
}

void TextureManager::Awake()
{
	InitTextureSamplers();
}

TextureManager & TextureManager::Allocate(Engine &engine)
{
	assert(mInstance == nullptr);
	mInstance = new TextureManager(engine);
	return *mInstance;
}

TextureManager & TextureManager::Get()
{
	return *mInstance;
}

VulkanImage * TextureManager::GetTexture(const char *pPath)
{
	auto it = mTextures.find(pPath);
	if (it != mTextures.end())
	{
		return it->second;
	}

	return nullptr;
}

VulkanImage * TextureManager::LoadTexture(const char *pPath)
{
	i32 width, height, channels;
	stbi_info(pPath, &width, &height, &channels);
	u64 requiredBytes = width * height * channels * 4;
	if (!HasEnoughMemory(requiredBytes))
	{
		throw std::runtime_error("Not enough memory to load texture.");
	}

	stbi_uc* pixels = stbi_load(pPath, &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels)
	{
	    throw std::runtime_error("Failed to load texture.");
	}

	VkExtent3D extent = { static_cast<u32>(width), static_cast<u32>(height), 1};
	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
	VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	auto newImage = CreateImage(mEngine, pixels, extent, format, usage, true);
	mTextures[pPath] = new VulkanImage(newImage);

	stbi_image_free(pixels);

	return mTextures[pPath];
}

VkSampler TextureManager::GetSampler(const std::string& name)
{
	if (mSamplers.contains(name))
	{
		return mSamplers[name];
	}

	return nullptr;
}

void TextureManager::InitTextureSamplers()
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(mEngine.mPhysicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

	VkSampler LINEAR_MIPMAP_LINEAR;
	vkCreateSampler(mEngine.mDevice, &samplerInfo, nullptr, &LINEAR_MIPMAP_LINEAR);
	mSamplers["LINEAR_MIPMAP_LINEAR"] = LINEAR_MIPMAP_LINEAR;

	samplerInfo.magFilter = VK_FILTER_NEAREST;
	samplerInfo.minFilter = VK_FILTER_NEAREST;

	VkSampler NEAREST_MIPMAP_LINEAR;
	vkCreateSampler(mEngine.mDevice, &samplerInfo, nullptr, &NEAREST_MIPMAP_LINEAR);
	mSamplers["NEAREST_MIPMAP_LINEAR"] = NEAREST_MIPMAP_LINEAR;
}
