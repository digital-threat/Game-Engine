#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

struct GpuMesh;

struct BlasInput
{
	std::vector<VkAccelerationStructureGeometryKHR> asGeometry;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR> asBuildOffsetInfo;
	VkBuildAccelerationStructureFlagsKHR flags;

	std::vector<u32> GetPrimitiveCounts()
	{
		std::vector<u32> maxPrimitiveCounts(asBuildOffsetInfo.size());
		for(u32 j = 0; j < asBuildOffsetInfo.size(); j++)
		{
			maxPrimitiveCounts[j] = asBuildOffsetInfo[j].primitiveCount;
		}
	}
};

struct AccelerationStructureBuildData
{
	VkAccelerationStructureTypeKHR asType = VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR;  // Mandatory to set

	std::vector<VkAccelerationStructureGeometryKHR> geometry;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR> rangeInfos;
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
	VkAccelerationStructureBuildSizesInfoKHR sizesInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
};


BlasInput MeshToVkGeometryKHR(GpuMesh& mesh);

class RaytracingBuilder
{
public:
	PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;

	RaytracingBuilder();

	void BuildBlas(std::vector<BlasInput>& input,
				VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);

	void BuildTlas(std::vector<VkAccelerationStructureInstanceKHR>& instances,
				VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);

private:
	Engine& mEngine;
	VkDevice mDevice;
	VkQueue mQueue;
	VmaAllocator mAllocator;
};
