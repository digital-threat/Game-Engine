#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>
#include <span>
#include <types.h>

namespace Renderer
{
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
}
