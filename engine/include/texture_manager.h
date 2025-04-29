#pragma once
#include <filesystem>
#include <string>
#include <texture_structs.h>
#include <types.h>
#include <unordered_map>
#include <vector>


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
	static TextureManager& Allocate(Engine& engine);
	static TextureManager& Instance();
	void LoadTexture(std::filesystem::path path, std::string& name);
	u32 GetTextureCount();
	VkSampler GetSampler(const std::string& name);

private:
	void InitTextureSamplers();
	std::unordered_map<std::string, VkSampler> GetSamplers();

public:
	std::vector<Texture> mTextures;

private:
	Engine& mEngine;
	static TextureManager* mInstance;
	std::unordered_map<std::string, VkSampler> mSamplers;
};
