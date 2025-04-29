#include <texture_manager.h>

#include <engine.h>
#include <stb_image.h>
#include <stdexcept>
#include <utility.h>
#include <vk_images.h>

TextureManager* TextureManager::mInstance = nullptr;

TextureManager::TextureManager(Engine& engine) : mEngine(engine) {}

void TextureManager::Awake() { InitTextureSamplers(); }

TextureManager& TextureManager::Allocate(Engine& engine)
{
	assert(mInstance == nullptr);
	mInstance = new TextureManager(engine);
	return *mInstance;
}

TextureManager& TextureManager::Instance() { return *mInstance; }

void TextureManager::LoadTexture(std::filesystem::path folder, std::string& name)
{
	std::filesystem::path path = folder / "textures" / name;

	i32 width, height, channels;
	stbi_info(path.string().c_str(), &width, &height, &channels);
	stbi_uc* pixels = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels)
	{
		throw std::runtime_error("Failed to load texture.");
	}

	VkExtent3D extent = {static_cast<u32>(width), static_cast<u32>(height), 1};
	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
	VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	auto image = CreateImage(mEngine, pixels, extent, format, usage, true);

	stbi_image_free(pixels);

	Texture texture;
	texture.image = image;
	texture.sampler = GetSampler("LINEAR_MIPMAP_LINEAR");
	mTextures.push_back(texture);
}

u32 TextureManager::GetTextureCount() { return mTextures.size(); }

VkSampler TextureManager::GetSampler(const std::string& name)
{
	if (mSamplers.contains(name))
	{
		return mSamplers[name];
	}

	return VK_NULL_HANDLE;
}

std::unordered_map<std::string, VkSampler> TextureManager::GetSamplers() { return mSamplers; }

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
