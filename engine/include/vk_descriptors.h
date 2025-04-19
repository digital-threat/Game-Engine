#pragma once

#include <deque>
#include <span>
#include <types.h>
#include <vector>
#include <vulkan/vulkan_core.h>

struct DescriptorLayoutBuilder
{
	std::vector<VkDescriptorSetLayoutBinding> mBindings;

	void AddBinding(u32 binding, VkDescriptorType type, VkShaderStageFlags stageFlags);
	void AddBinding(u32 binding, VkDescriptorType type, u32 count, VkShaderStageFlags stageFlags);
	void Clear();
	VkDescriptorSetLayout Build(VkDevice device, const void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);
};

struct DescriptorAllocator
{
	struct PoolSizeRatio
	{
		VkDescriptorType type;
		float ratio; // TODO(Sergei): Why does this have to be a float?
	};

	VkDescriptorPool mPool;

	void InitializePool(VkDevice device, u32 maxSets, std::span<PoolSizeRatio> poolRatios);
	void ClearDescriptors(VkDevice device);
	void DestroyPool(VkDevice device);
	VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout layout);
};

struct DescriptorWriter
{
	std::deque<VkDescriptorImageInfo> mImageInfos;
	std::deque<VkDescriptorBufferInfo> mBufferInfos;
	std::vector<VkWriteDescriptorSet> mWrites;

	void WriteImage(u32 binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type);
	void WriteBuffer(u32 binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type);
	void WriteTlas(u32 binding, VkWriteDescriptorSetAccelerationStructureKHR* writeAS);

	void Clear();
	void UpdateSet(VkDevice device, VkDescriptorSet set);
};
