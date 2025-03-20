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

BlasInput MeshToVkGeometryKHR(const GpuMesh& mesh);

class RaytracingBuilder
{
public:

	void BuildBlas(const std::vector<BlasInput>& input,
				VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);

	void BuildTlas(const std::vector<VkAccelerationStructureInstanceKHR>& instances,
				VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);

};
