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
		for(u32 j = 0; j < asBuildOffsetInfo.size(); j++)
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
	VkDeviceAddress address;
};


BlasInput MeshToVkGeometryKHR(GpuMesh& mesh);

class RaytracingBuilder
{
public:
	PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
	PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
	PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;

	RaytracingBuilder() = delete;
	explicit RaytracingBuilder(Engine& engine);

	void BuildBlas(std::vector<BlasInput>& input,
				VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);

	void BuildTlas(std::vector<VkAccelerationStructureInstanceKHR>& instances,
				VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);

private:

	std::vector<AccelerationStructure> mBlas;

	Engine& mEngine;
	VkDevice mDevice;
	VkQueue mQueue;
	VmaAllocator mAllocator;
};
