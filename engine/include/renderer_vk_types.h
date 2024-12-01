#pragma once

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>

namespace Renderer
{
	struct VulkanImage
	{
		VkImage image;
		VkImageView imageView;
		VmaAllocation allocation;
		VkExtent3D extent;
		VkFormat format;
	};

	struct VulkanBuffer
	{
    	VkBuffer buffer;
    	VmaAllocation allocation;
    	VmaAllocationInfo info;
	};

	struct GPUMeshBuffers
	{
		VulkanBuffer indexBuffer;
		VulkanBuffer vertexBuffer;
		VkDeviceAddress vertexBufferAddress;
	};

	struct GPUDrawPushConstants
	{
		glm::mat4 worldMatrix;
		VkDeviceAddress vertexBuffer;
	};
}

