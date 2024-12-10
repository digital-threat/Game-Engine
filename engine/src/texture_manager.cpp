#include "texture_manager.h"

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
	auto newImage = CreateImage(mEngine, pixels, extent, format, usage);
	mTextures[pPath] = new VulkanImage(newImage);

	stbi_image_free(pixels);

	return mTextures[pPath];
}
