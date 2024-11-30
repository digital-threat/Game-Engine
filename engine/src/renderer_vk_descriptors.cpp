#include "renderer_vk_descriptors.h"

#include <stdexcept>

void Renderer::DescriptorLayoutBuilder::AddBinding(u32 pBinding, VkDescriptorType pType)
{
	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding = pBinding;
	layoutBinding.descriptorCount = 1;
	layoutBinding.descriptorType = pType;

	mBindings.push_back(layoutBinding);
}

void Renderer::DescriptorLayoutBuilder::Clear()
{
	mBindings.clear();
}

VkDescriptorSetLayout Renderer::DescriptorLayoutBuilder::Build(const VkDevice pDevice, const VkShaderStageFlags pShaderStages, const void *pNext, const VkDescriptorSetLayoutCreateFlags pFlags)
{
	for (auto& binding : mBindings)
	{
		binding.stageFlags |= pShaderStages;
	}

	VkDescriptorSetLayoutCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.pNext = pNext;
	info.pBindings = mBindings.data();
	info.bindingCount = static_cast<u32>(mBindings.size());
	info.flags = pFlags;

	VkDescriptorSetLayout set;
	if (vkCreateDescriptorSetLayout(pDevice, &info, nullptr, &set) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout.");
	}

	return set;
}

void Renderer::DescriptorAllocator::InitializePool(const VkDevice pDevice, const u32 pMaxSets, std::span<PoolSizeRatio> pPoolRatios)
{
	std::vector<VkDescriptorPoolSize> poolSizes;
	poolSizes.reserve(pPoolRatios.size());

	for (PoolSizeRatio ratio : pPoolRatios)
	{
		poolSizes.emplace_back(ratio.type,static_cast<u32>(ratio.ratio * pMaxSets));
	}

	VkDescriptorPoolCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	info.flags = 0;
	info.maxSets = pMaxSets;
	info.poolSizeCount = static_cast<u32>(poolSizes.size());
	info.pPoolSizes = poolSizes.data();

	vkCreateDescriptorPool(pDevice, &info, nullptr, &mPool);
}

void Renderer::DescriptorAllocator::ClearDescriptors(const VkDevice pDevice)
{
	vkResetDescriptorPool(pDevice, mPool, 0);
}

void Renderer::DescriptorAllocator::DestroyPool(const VkDevice pDevice)
{
	vkDestroyDescriptorPool(pDevice, mPool, nullptr);
}

VkDescriptorSet Renderer::DescriptorAllocator::Allocate(const VkDevice pDevice, const VkDescriptorSetLayout pLayout)
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.descriptorPool = mPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &pLayout;

	VkDescriptorSet descriptorSet;
	if (vkAllocateDescriptorSets(pDevice, &allocInfo, &descriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate descriptor sets.");
	}

	return descriptorSet;
}
