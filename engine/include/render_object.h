#pragma once

#include <types.h>
#include <vk_structs.h>
#include <string>
#include <glm/mat4x4.hpp>

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