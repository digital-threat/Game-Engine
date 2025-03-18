#pragma once

#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>

#include <deque>
#include <functional>
#include <string>
#include <array>

#include <types.h>
#include <vk_descriptors.h>
#include <vk_structs.h>
#include <lights.h>

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
