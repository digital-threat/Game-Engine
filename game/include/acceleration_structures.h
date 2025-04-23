#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <string.h>
#include <types.h>
#include <vector>
#include <vk_structs.h>
#include <vulkan/vulkan_core.h>

struct GpuMesh;
class Engine;

struct BLAS
{
	VkAccelerationStructureKHR handle;
	VulkanBuffer buffer;
	VkDeviceAddress deviceAddress;
};

// NOTE(Sergei): Scratch buffer is reused for updating tlas.
struct TLAS
{
	VkAccelerationStructureKHR handle;
	VulkanBuffer buffer;
	VulkanBuffer scratchBuffer;
	VkDeviceAddress scratchAddress;
	VulkanBuffer instanceBuffer;
	VkDeviceAddress instanceBufferAddress;
	u32 instanceCount;
};

struct Scene
{
	TLAS tlas;
	std::vector<BLAS> blas;

	VkDeviceAddress GetBlasDeviceAddress(u32 index)
	{
		assert(index < blas.size());
		return blas[index].deviceAddress;
	}
};

struct BlasInput
{
	std::vector<VkAccelerationStructureGeometryKHR> asGeometry;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR> asBuildOffsetInfo;
	VkBuildAccelerationStructureFlagsKHR flags;

	std::vector<u32> GetPrimitiveCounts()
	{
		std::vector<u32> maxPrimitiveCounts(asBuildOffsetInfo.size());
		for (u32 j = 0; j < asBuildOffsetInfo.size(); j++)
		{
			maxPrimitiveCounts[j] = asBuildOffsetInfo[j].primitiveCount;
		}

		return maxPrimitiveCounts;
	}
};

BlasInput MeshToVkGeometryKHR(GpuMesh& mesh);

class RaytracingBuilder
{
	struct AccelerationStructureBuildData
	{
		std::vector<VkAccelerationStructureBuildRangeInfoKHR> rangeInfos;
		VkAccelerationStructureBuildGeometryInfoKHR geometryInfo;
		VkAccelerationStructureBuildSizesInfoKHR sizesInfo;
	};

public:
	RaytracingBuilder() = delete;
	explicit RaytracingBuilder(Engine& engine);

	void BuildBlas(std::vector<BlasInput>& input, std::vector<BLAS>& output,
				   VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);

	void BuildTlas(std::vector<VkAccelerationStructureInstanceKHR>& instances, TLAS& tlas,
				   VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);

	void UpdateTlas(TLAS& tlas,
					VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);

private:
	Engine& mEngine;
};


inline VkTransformMatrixKHR ToVkTransformMatrixKHR(glm::mat4 matrix)
{
	// NOTE(Sergei): Transpose because GLM is column-major and VkTransformMatrixKHR is row-major
	glm::mat4 transposed = glm::transpose(matrix);
	VkTransformMatrixKHR result;
	memcpy(&result, &transposed, sizeof(VkTransformMatrixKHR));
	return result;
}
