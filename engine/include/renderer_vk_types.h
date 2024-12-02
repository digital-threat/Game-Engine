#pragma once

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "types.h"

namespace Renderer
{
	struct Vertex
	{
		glm::vec3 position;
		float uvX;
		glm::vec3 normal;
		float uvY;
		glm::vec4 color;
	};

	struct Submesh
	{
		u32 startIndex;
		size_t count;
	};

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

	struct MeshAsset
	{
		std::string name;
		std::vector<Submesh> submeshes;
		GPUMeshBuffers meshBuffers;
	};
}

