#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

struct GpuMesh;

struct BlasInput
{
	std::vector<VkAccelerationStructureGeometryKHR>       asGeometry;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR> asBuildOffsetInfo;
	VkBuildAccelerationStructureFlagsKHR                  flags{0};
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
	VkDevice device;
	VmaAllocator allocator;
};
