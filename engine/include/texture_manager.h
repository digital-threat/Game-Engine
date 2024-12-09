#pragma once
#include <renderer_vk_types.h>

class Engine;
using namespace Renderer;

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
	static TextureManager& Allocate(Engine &engine);
	static TextureManager& Get();
	VulkanImage* GetTexture(const char* pPath);
	VulkanImage* LoadTexture(const char *pPath);

private:
	Engine& mEngine;
	static TextureManager* mInstance;
	std::unordered_map<const char*, VulkanImage*> mTextures;
};