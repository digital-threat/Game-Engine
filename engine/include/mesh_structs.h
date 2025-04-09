#pragma once

#include <material_structs.h>
#include <vk_structs.h>
#include <vertex.h>
#include <types.h>
#include <vector>

typedef u32 MeshHandle;

struct CpuMesh
{
	std::vector<Vertex> vertices;
	std::vector<u32> indices;
	std::vector<Material> materials;
	std::vector<i32> matIds;
	u32 textureOffset;

};

// NOTE(Sergei): I wonder if having just one big buffer of materials is worth it.
// (as opposed to each mesh having its own materials buffer)
// I don't think it'll save much memory, but are there other factors to consider?
struct GpuMesh
{
	u32 textureOffset;
	u32 indexCount;
	u32 vertexCount;
	VulkanBuffer indexBuffer;
	VulkanBuffer vertexBuffer;
	VulkanBuffer materialBuffer;
	VulkanBuffer matIdBuffer;
	VkDeviceAddress vertexBufferAddress;
	VkDeviceAddress indexBufferAddress;
	VkDeviceAddress materialBufferAddress;
	VkDeviceAddress matIdBufferAddress;
};