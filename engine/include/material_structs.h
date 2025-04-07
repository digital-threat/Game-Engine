#pragma once

#include <vulkan/vulkan_core.h>
#include <string>
#include <texture_structs.h>
#include <types.h>
#include <glm/vec3.hpp>
#include <vk_images.h>

struct MaterialHandle
{
	u16 index = -1;
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

struct GpuMaterial
{
	VulkanImage albedoImage;
	VkSampler albedoSampler;
	VkBuffer dataBuffer;
	uint32_t dataBufferOffset;
};

struct WaveformMaterial
{
	glm::vec3  ambient;
	glm::vec3  diffuse;
	glm::vec3  specular;
	glm::vec3  transmittance;
	glm::vec3  emission;
	float shininess;
	float ior;
	float dissolve;
	int   illum;
};

struct CpuMaterial
{
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 transmittance;
	glm::vec3 emission;
	float shininess;
	float ior;       // index of refraction
	float dissolve;  // 1 == opaque; 0 == fully transparent
	int illum;     // illumination model (see http://www.fileformat.info/format/material/)
	std::string diffuseTexture;
};
