#pragma once

#include <camera.h>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>

#include <deque>
#include <functional>
#include <handles.h>
#include <string>
#include <vector>

#include "types.h"
#include "renderer_vk_descriptors.h"


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

struct Vertex
{
	glm::vec3 position;
	float u;
	glm::vec3 normal;
	float v;

	bool operator==(const Vertex& other) const
	{
		return position == other.position &&
			   u == other.u &&
			   v == other.v &&
			   normal == other.normal;
	}
};

enum class LightType : u8
{
	DIRECTIONAL,
	POINT,
	SPOT,
};

struct Light
{
	glm::vec3 direction;
	float attenuation;
	glm::vec3 color;
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

struct MeshData
{
	std::string name;
	std::vector<Vertex> vertices;
	std::vector<u32> indices;
};

struct MeshAsset
{
	std::string name;
	u32 indexCount;
	//std::vector<Submesh> submeshes;
	MeshBuffers meshBuffers;
};

enum class RenderQueue : u8
{
	OPAQUE,
	TRANSPARENT,
};

struct GeometryPushConstants
{
	glm::mat4 matrixM;
	glm::mat4 matrixITM;
	VkDeviceAddress vertexBuffer;
};

struct ComputePushConstants
{
	glm::vec4 data1;
	glm::vec4 data2;
	glm::vec4 data3;
	glm::vec4 data4;
};

struct FrameData
{
	VkSemaphore swapchainSemaphore, renderSemaphore;
	VkFence renderFence;

	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;

	VulkanBuffer sceneDataBuffer;

	DeletionQueue deletionQueue;
	DescriptorAllocator descriptorAllocator;
};

struct SceneData
{
	glm::mat4 matrixV;
	glm::mat4 matrixP;
	glm::mat4 matrixVP;
	glm::vec4 ambientColor;
	glm::vec4 mainLightDir;
	glm::vec4 mainLightColor;
	glm::vec4 cameraPos;
};

struct Texture
{
	VkImageView view;
	VkSampler sampler;
};

struct Material
{
	std::string name;
	MaterialHandle handle;
	VkBuffer dataBuffer;
	float shininess;
	VkDescriptorSet materialSet;
};

struct ModelRenderData
{
	std::string name;
	u32 indexCount;
	VulkanBuffer indexBuffer;
	VulkanBuffer vertexBuffer;
	VkDeviceAddress vertexBufferAddress;
	glm::mat4 transform;
	VkDescriptorSet materialSet;
};

struct SceneRenderData
{
	glm::vec3 ambientColor;
	glm::vec3 mainLightDir;
	glm::vec3 mainLightColor;
	glm::vec3 cameraPos;
	glm::vec3 cameraLookAt;
	float cameraFOV;
};

struct ImmediateData
{
	VkFence fence = VK_NULL_HANDLE;
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	VkCommandPool cmdPool = VK_NULL_HANDLE;
};

struct RenderContext
{
	std::vector<ModelRenderData> modelData;
	SceneRenderData sceneData;
	float renderScale = 1.0f;
};