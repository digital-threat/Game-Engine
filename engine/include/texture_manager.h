#pragma once
#include <vk_structs.h>
#include <unordered_map>
#include <string>

class Engine;

class TextureManager
{
public:
	TextureManager() = delete;
	TextureManager(const TextureManager&) = delete;
	TextureManager(TextureManager&&) = delete;

	TextureManager& operator=(const TextureManager&) = delete;
	TextureManager& operator=(TextureManager&&) = delete;

	explicit TextureManager(Engine& engine);
	
public:
	void Awake();
	static TextureManager& Allocate(Engine &engine);
	static TextureManager& Get();
	VulkanImage* GetTexture(const char* pPath);
	VulkanImage* LoadTexture(const char *pPath);
	VkSampler GetSampler(const std::string& name);
	std::unordered_map<std::string, VkSampler> GetSamplers();

private:
	void InitTextureSamplers();

private:
	Engine& mEngine;
	static TextureManager* mInstance;
	std::unordered_map<const char*, VulkanImage*> mImages;
	std::unordered_map<std::string, VkSampler> mSamplers;
};