#pragma once
#include <material_structs.h>
#include <types.h>
#include <vector>

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
	MaterialHandle CreateMaterial(const std::string &name);
	const std::vector<Material>& GetAll();

	void SetTexture(MaterialHandle handle, Texture texture, u32 slot);
	VkDescriptorSet GetDescriptorSet(MaterialHandle handle);

private:
	Engine& mEngine;
	static MaterialManager* mInstance;

	std::vector<Material> mMaterials;
	//std::vector<GpuMaterial> mMaterials;
	const u16 MAX_MATERIALS = 256;
};
