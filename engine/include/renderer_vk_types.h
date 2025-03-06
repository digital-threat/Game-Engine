#pragma once

#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>

#include <deque>
#include <functional>
#include <handles.h>
#include <string>
#include <vector>
#include <array>

#include <types.h>
#include <renderer_vk_descriptors.h>
#include <vulkan_structs.h>

#define MAX_LIGHTS 8

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



enum class LightType : u8
{
	DIRECTIONAL,
	POINT,
	SPOT,
};

struct LightData
{
	glm::vec4 position;
	glm::vec4 color;
	glm::vec4 spotDirection;
	glm::vec4 attenuation;
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

struct ShadowmapPushConstants
{
	glm::mat4 depthMVP;
	VkDeviceAddress vertexBuffer;
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
	glm::mat4 mainLightVP;
	glm::vec4 mainLightDir;
	glm::vec4 mainLightColor;
	glm::vec4 ambientColor;
	glm::vec4 cameraPos;
	std::array<LightData, MAX_LIGHTS> lightBuffer;
	u32 lightCount;
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
	Texture textures[2];
	VkDescriptorSet materialSet;
};

struct RenderObject
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
	glm::vec3 mainLightPos;
	glm::vec4 mainLightColor;
	glm::vec3 ambientColor;
	glm::vec3 cameraPos;
	glm::vec3 cameraLookAt;
	float cameraFOV;
};

struct LightRenderData
{
	std::array<LightData, MAX_LIGHTS> lightBuffer;
	u32 lightCount = 0;
};

struct ImmediateData
{
	VkFence fence = VK_NULL_HANDLE;
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	VkCommandPool cmdPool = VK_NULL_HANDLE;
};

struct RenderContext
{
	std::vector<RenderObject> renderObjects;
	SceneRenderData sceneData;
	LightRenderData lightData;
	float renderScale = 1.0f;
};