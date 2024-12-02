#pragma once

#include <deque>
#include <functional>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "types.h"


namespace Renderer
{
	// TODO(Sergei): Temporary, replace!
	struct DeletionQueue
	{
		std::deque<std::function<void()>> deletors;

		void Push(std::function<void()>&& function)
		{
			deletors.push_back(function);
		}

		void Flush()
		{
			for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
			{
				(*it)();
			}

			deletors.clear();
		}
	};

	struct FrameData
	{
		VkSemaphore swapchainSemaphore, renderSemaphore;
		VkFence renderFence;

		VkCommandPool commandPool;
		VkCommandBuffer mainCommandBuffer;

		DeletionQueue deletionQueue;
	};

	struct Vertex
	{
		glm::vec3 position;
		float uvX;
		glm::vec3 normal;
		float uvY;
		glm::vec4 color;
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

	struct MeshBuffers
	{
		VulkanBuffer indexBuffer;
		VulkanBuffer vertexBuffer;
		VkDeviceAddress vertexBufferAddress;
	};

	struct Submesh
	{
		u32 startIndex;
		size_t count;
	};

	struct MeshAsset
	{
		std::string name;
		std::vector<Submesh> submeshes;
		MeshBuffers meshBuffers;
	};

	struct GeometryPushConstants
	{
		glm::mat4 worldMatrix;
		VkDeviceAddress vertexBuffer;
	};

	struct ComputePushConstants
	{
		glm::vec4 data1;
		glm::vec4 data2;
		glm::vec4 data3;
		glm::vec4 data4;
	};
}

