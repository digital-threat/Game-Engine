#include "material_manager.h"

#include <engine.h>

MaterialManager* MaterialManager::mInstance = nullptr;

MaterialManager::MaterialManager(Engine &engine)
	: mEngine(engine)
{
	mMaterials.reserve(MAX_MATERIALS);
}

MaterialManager& MaterialManager::Allocate(Engine &engine)
{
	assert(mInstance == nullptr);
	mInstance = new MaterialManager(engine);
	return *mInstance;
}

MaterialManager& MaterialManager::Get()
{
	return *mInstance;
}

// MaterialHandle MaterialManager::CreateMaterial(const std::string &name)
// {
// 	assert(mMaterials.size() < MAX_MATERIALS);
// 	MaterialHandle handle = { static_cast<u16>(mMaterials.size()) };
// 	auto& instance = mMaterials.emplace_back();
// 	instance.name = name;
// 	instance.handle = handle;
// 	instance.materialSet = mEngine.mGlobalDescriptorAllocator.Allocate(mEngine.mDevice, mEngine.mMaterialDescriptorLayout);
// 	return handle;
// }
//
// void MaterialManager::WriteMaterial(Material& material)
// {
//
// }
//
// const std::vector<Material_> & MaterialManager::GetAll()
// {
// 	return mMaterials;
// }
//
// void MaterialManager::SetTexture(MaterialHandle handle, Texture texture, u32 slot)
// {
// 	DescriptorWriter writer;
// 	if (texture.view != VK_NULL_HANDLE && texture.sampler == VK_NULL_HANDLE)
// 	{
// 		writer.WriteImage(slot, texture.view, mMaterials[handle.index].textures[slot].sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
// 		mMaterials[handle.index].textures[slot].view = texture.view;
// 	}
// 	else if (texture.view == VK_NULL_HANDLE && texture.sampler != VK_NULL_HANDLE)
// 	{
// 		writer.WriteImage(slot, mMaterials[handle.index].textures[slot].view, texture.sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
// 		mMaterials[handle.index].textures[slot].sampler = texture.sampler;
// 	}
// 	else if (texture.view != VK_NULL_HANDLE && texture.sampler != VK_NULL_HANDLE)
// 	{
// 		writer.WriteImage(slot, texture.view, texture.sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
// 		mMaterials[handle.index].textures[slot].view = texture.view;
// 		mMaterials[handle.index].textures[slot].sampler = texture.sampler;
// 	}
// 	else
// 	{
// 		return;
// 	}
//
// 	writer.UpdateSet(mEngine.mDevice, mMaterials[handle.index].materialSet);
// }
//
// VkDescriptorSet MaterialManager::GetDescriptorSet(MaterialHandle handle)
// {
// 	if (handle.index >= mMaterials.size() || handle.index < 0)
// 	{
// 		return VK_NULL_HANDLE;
// 	}
// 	return mMaterials[handle.index].materialSet;
// }
