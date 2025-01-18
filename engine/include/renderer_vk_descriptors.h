#pragma once

#include <deque>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <span>
#include <types.h>

struct DescriptorLayoutBuilder
{
	std::vector<VkDescriptorSetLayoutBinding> mBindings;

	void AddBinding(u32 pBinding, VkDescriptorType pType);
	void Clear();
	VkDescriptorSetLayout Build(VkDevice pDevice, VkShaderStageFlags pShaderStages, const void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags pFlags = 0);
};

struct DescriptorAllocator
{
	struct PoolSizeRatio
	{
		VkDescriptorType type;
		float ratio; // TODO(Sergei): Why does this have to be a float?
	};

	VkDescriptorPool mPool;

	void InitializePool(VkDevice pDevice, u32 pMaxSets, std::span<PoolSizeRatio> pPoolRatios);
	void ClearDescriptors(VkDevice pDevice);
	void DestroyPool(VkDevice pDevice);
	VkDescriptorSet Allocate(VkDevice pDevice, VkDescriptorSetLayout pLayout);
};

struct DescriptorWriter
{
	std::deque<VkDescriptorImageInfo> mImageInfos;
	std::deque<VkDescriptorBufferInfo> mBufferInfos;
	std::vector<VkWriteDescriptorSet> mWrites;

	void WriteImage(u32 pBinding, VkImageView pImage, VkSampler pSampler, VkImageLayout pLayout, VkDescriptorType pType);
	void WriteBuffer(u32 pBinding, VkBuffer pBuffer, size_t pSize, size_t pOffset, VkDescriptorType pType);

	void Clear();
	void UpdateSet(VkDevice pDevice, VkDescriptorSet pSet);
};
