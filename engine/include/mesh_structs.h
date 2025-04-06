#pragma once

#include <material_structs.h>
#include <vk_structs.h>
#include <vertex.h>
#include <types.h>

#include <string>
#include <vector>

struct CpuMesh
{
	std::string name;
	std::vector<Vertex> vertices;
	std::vector<u32> indices;
	std::vector<u32> matIds;
};

struct GpuMesh
{
	u32 indexCount;
	u32 vertexCount;
	VulkanBuffer indexBuffer;
	VulkanBuffer vertexBuffer;
	VkDeviceAddress vertexBufferAddress;
	VkDeviceAddress indexBufferAddress;
};