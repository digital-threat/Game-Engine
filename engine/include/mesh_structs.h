#pragma once

#include <vk_structs.h>
#include <vertex.h>
#include <types.h>

#include <string>
#include <vector>

struct Submesh
{
	u32 startIndex;
	size_t count;
};

struct CpuMesh
{
	std::string name;
	std::vector<Vertex> vertices;
	std::vector<u32> indices;
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