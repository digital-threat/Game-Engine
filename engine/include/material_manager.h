#pragma once
#include <renderer_vk_types.h>
#include <handles.h>

class Engine;

class MaterialManager
{
public:
	MaterialManager() = delete;
	MaterialManager(const MaterialManager&) = delete;
	MaterialManager(MaterialManager&&) = delete;

	MaterialManager& operator=(const MaterialManager&) = delete;
	MaterialManager& operator=(MaterialManager&&) = delete;

	explicit MaterialManager(Engine& engine);

public:
	static MaterialManager& Allocate(Engine &engine);
	static MaterialManager& Get();

public:
	MaterialHandle CreateMaterial();

	void SetTexture(MaterialHandle handle, Texture texture, u32 slot);
	VkDescriptorSet GetDescriptorSet(MaterialHandle handle);

private:
	Engine& mEngine;
	static MaterialManager* mInstance;

	std::vector<MaterialInstance> mMaterials;
	const u16 MAX_MATERIALS = 256;
};
