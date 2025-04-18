#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

struct GpuMesh;
class Engine;

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

struct AccelerationStructureBuildData
{
	std::vector<VkAccelerationStructureBuildRangeInfoKHR> rangeInfos;
	VkAccelerationStructureBuildGeometryInfoKHR geometryInfo;
	VkAccelerationStructureBuildSizesInfoKHR sizesInfo;
};

struct AccelerationStructure
{
	VkAccelerationStructureKHR handle;
	VulkanBuffer buffer;
	VkDeviceAddress deviceAddress;
};


BlasInput MeshToVkGeometryKHR(GpuMesh& mesh);

class RaytracingBuilder
{
public:
	RaytracingBuilder() = delete;
	explicit RaytracingBuilder(Engine& engine);

	void BuildBlas(std::vector<BlasInput>& input,
				   VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
	void BuildTlas(std::vector<VkAccelerationStructureInstanceKHR>& instances,
				   VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
				   bool update = false);

	VkDeviceAddress GetBlasDeviceAddress(u32 index);

public:
	AccelerationStructure mTlas;

private:
	std::vector<AccelerationStructure> mBlas;

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
