#pragma once

#include <vk_structs.h>
#include <vertex.h>
#include <types.h>

#include <string>
#include <vector>

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

struct Mesh
{
	u32 indexCount;
	MeshBuffers meshBuffers;
};