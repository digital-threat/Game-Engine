#pragma once

#include <mesh_structs.h>
#include <glm/mat4x4.hpp>

struct RenderInstance
{
	glm::mat4 transform;
	MeshHandle meshHandle;
};

struct ObjectData
{
	VkDeviceAddress vertexBufferAddress;
	VkDeviceAddress indexBufferAddress;
	VkDeviceAddress materialBufferAddress;
	VkDeviceAddress matIdBufferAddress;
	u32 textureOffset;
};