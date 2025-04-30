#pragma once

#include <glm/vec3.hpp>
#include <string>
#include <texture_structs.h>
#include <types.h>
#include <vk_images.h>
#include <vulkan/vulkan_core.h>

struct MaterialHandle
{
	u16 index = -1;
};

struct GpuMaterial
{
	VulkanImage albedoImage;
	VkSampler albedoSampler;
	VkBuffer dataBuffer;
	uint32_t dataBufferOffset;
};

struct Material
{
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 transmittance;
	glm::vec3 emission;
	float shininess;
	float ior;
	float dissolve;
	int illum;
	int diffuseTextureIndex;
	int alphaTextureIndex;
	int specularTextureIndex;
};
