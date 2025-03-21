#include <mesh_structs.h>
#include <vertex.h>
#include <vk_buffers.h>
#include <vk_raytracing.h>

BlasInput MeshToVkGeometryKHR(GpuMesh& mesh)
{
	VkAccelerationStructureGeometryTrianglesDataKHR triangles{};
	triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	triangles.vertexData.deviceAddress = mesh.vertexBufferAddress;
	triangles.vertexStride = sizeof(Vertex);
	triangles.indexType = VK_INDEX_TYPE_UINT32;
	triangles.indexData.deviceAddress = mesh.indexBufferAddress;
	//triangles.transformData = {};
	triangles.maxVertex = mesh.vertexCount - 1;

	VkAccelerationStructureGeometryKHR asGeometry{};
	asGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	asGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	asGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	asGeometry.geometry.triangles = triangles;

	VkAccelerationStructureBuildRangeInfoKHR offset;
	offset.firstVertex = 0;
	offset.primitiveCount = mesh.indexCount / 3;
	offset.primitiveOffset = 0;
	offset.transformOffset = 0;

	BlasInput input;
	input.asGeometry.emplace_back(asGeometry);
	input.asBuildOffsetInfo.emplace_back(offset);
	return input;
}

RaytracingBuilder::RaytracingBuilder()
{
	vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));
}

void RaytracingBuilder::BuildBlas(std::vector<BlasInput>& input, VkBuildAccelerationStructureFlagsKHR flags)
{
	VkDeviceSize asTotalSize{0};
	VkDeviceSize maxScratchSize{0};

	u32 blasCount = static_cast<u32>(input.size());
	for (u32 i = 0; i < blasCount; i++)
	{
		VkAccelerationStructureBuildGeometryInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		info.flags = input[i].flags | flags; // VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		info.geometryCount = static_cast<u32>(input[i].asGeometry.size());
		info.pGeometries = input[i].asGeometry.data();

		// Range info
		std::vector<VkAccelerationStructureBuildRangeInfoKHR> rangeInfos = input[i].asBuildOffsetInfo;

		// Sizes info
		VkAccelerationStructureBuildSizesInfoKHR sizesInfo{};
		sizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

		std::vector<u32> maxPrimitiveCounts(input[i].asBuildOffsetInfo.size());
		for(u32 j = 0; j < input[i].asBuildOffsetInfo.size(); j++)
		{
			maxPrimitiveCounts[j] = input[i].asBuildOffsetInfo[j].primitiveCount;
		}

		auto buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
		vkGetAccelerationStructureBuildSizesKHR(device, buildType, &info, maxPrimitiveCounts.data(), &sizesInfo);

		asTotalSize += sizesInfo.accelerationStructureSize;
		maxScratchSize = std::max(maxScratchSize, sizesInfo.buildScratchSize);
	}

	VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	VulkanBuffer scratchBuffer = CreateBuffer(allocator, maxScratchSize, bufferUsage, VMA_MEMORY_USAGE_AUTO);
	VkBufferDeviceAddressInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	bufferInfo.buffer = scratchBuffer.buffer;
	VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(device, &bufferInfo);


	// VkCommandBuffer cmdBuf = m_cmdPool.createCommandBuffer();
	// cmdCreateBlas(cmdBuf, indices, buildAs, scratchAddress, queryPool);
	// m_cmdPool.submitAndWait(cmdBuf);

	// for(auto& b : buildAs)
	// {
	// 	m_blas.emplace_back(b.as);
	// }

	DestroyBuffer(allocator, scratchBuffer);
}

void RaytracingBuilder::BuildTlas(std::vector<VkAccelerationStructureInstanceKHR>& instances,
	VkBuildAccelerationStructureFlagsKHR flags)
{
}
