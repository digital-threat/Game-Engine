#include <acceleration_structures.h>
#include <engine.h>
#include <mesh_structs.h>
#include <utility.h>
#include <vertex.h>
#include <vk_buffers.h>
#include <vk_immediate.h>

BlasInput MeshToVkGeometryKHR(GpuMesh& mesh)
{
	VkAccelerationStructureGeometryTrianglesDataKHR triangles{};
	triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	triangles.vertexData.deviceAddress = mesh.vertexBufferAddress;
	triangles.vertexStride = sizeof(Vertex);
	triangles.indexType = VK_INDEX_TYPE_UINT32;
	triangles.indexData.deviceAddress = mesh.indexBufferAddress;
	// triangles.transformData = {};
	triangles.maxVertex = mesh.vertexCount - 1;

	VkAccelerationStructureGeometryKHR asGeometry{};
	asGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	asGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	asGeometry.flags = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;
	asGeometry.geometry.triangles = triangles;

	VkAccelerationStructureBuildRangeInfoKHR offset;
	offset.firstVertex = 0;
	offset.primitiveCount = mesh.indexCount / 3;
	offset.primitiveOffset = 0;
	offset.transformOffset = 0;

	BlasInput input{};
	input.asGeometry.emplace_back(asGeometry);
	input.asBuildOffsetInfo.emplace_back(offset);
	return input;
}

RaytracingBuilder::RaytracingBuilder(Engine& engine) : mEngine(engine) {}

// TODO(Sergei): Compact BLAS (could save over 50% memory)
// TODO(Sergei): Allocate one big scratch buffer and use regions of it to build different BLAS
// TODO(Sergei): Query AS compacted size in bulk
void RaytracingBuilder::BuildBlas(std::vector<BlasInput>& input, std::vector<BLAS>& output, VkBuildAccelerationStructureFlagsKHR flags)
{
	output.resize(input.size());

	VkDeviceSize maxScratchSize = 0;

	std::vector<AccelerationStructureBuildData> buildData;
	buildData.resize(input.size());

	u32 blasCount = static_cast<u32>(input.size());
	for (u32 i = 0; i < blasCount; i++)
	{
		// Build geometry info
		VkAccelerationStructureBuildGeometryInfoKHR geometryInfo{};
		geometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		geometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		geometryInfo.flags = input[i].flags | flags;
		geometryInfo.geometryCount = static_cast<u32>(input[i].asGeometry.size());
		geometryInfo.pGeometries = input[i].asGeometry.data();
		buildData[i].geometryInfo = geometryInfo;

		// Build range info
		std::vector<VkAccelerationStructureBuildRangeInfoKHR> rangeInfos = input[i].asBuildOffsetInfo;
		buildData[i].rangeInfos = rangeInfos;

		// Build sizes info
		VkAccelerationStructureBuildSizesInfoKHR sizesInfo{};
		sizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		std::vector<u32> primitiveCounts = input[i].GetPrimitiveCounts();
		auto buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
		mEngine.mVkbDT.fp_vkGetAccelerationStructureBuildSizesKHR(mEngine.mDevice, buildType, &geometryInfo, primitiveCounts.data(),
																  &sizesInfo);
		buildData[i].sizesInfo = sizesInfo;

		maxScratchSize = std::max(maxScratchSize, sizesInfo.buildScratchSize);
	}

	// Create scratch buffer
	VkBufferUsageFlags scratchBufferUsage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	VulkanBuffer scratchBuffer =
			CreateBufferAligned(mEngine.mAllocator, maxScratchSize, scratchBufferUsage, VMA_MEMORY_USAGE_GPU_ONLY, 128);

	// Get device address of scratch buffer
	VkBufferDeviceAddressInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	bufferInfo.buffer = scratchBuffer.buffer;
	VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(mEngine.mDevice, &bufferInfo);

	// Create query pool
	VkQueryPoolCreateInfo queryPoolCreateInfo{};
	queryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	queryPoolCreateInfo.queryCount = blasCount;
	queryPoolCreateInfo.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR;

	VkQueryPool queryPool;
	vkCreateQueryPool(mEngine.mDevice, &queryPoolCreateInfo, nullptr, &queryPool);

	std::vector<BLAS> tempBlas(blasCount);
	for (u32 i = 0; i < blasCount; i++)
	{
		auto func = [&](VkCommandBuffer cmd)
		{
			// Create acceleration structure buffer
			VkBufferUsageFlags asBufferUsage =
					VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
			tempBlas[i].buffer = CreateBuffer(mEngine.mAllocator, buildData[i].sizesInfo.accelerationStructureSize, asBufferUsage,
											  VMA_MEMORY_USAGE_GPU_ONLY);

			// Create info
			VkAccelerationStructureCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
			createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			createInfo.size = buildData[i].sizesInfo.accelerationStructureSize;
			createInfo.buffer = tempBlas[i].buffer.buffer;
			mEngine.mVkbDT.fp_vkCreateAccelerationStructureKHR(mEngine.mDevice, &createInfo, nullptr, &tempBlas[i].handle);

			// Build geometry info (cont.)
			buildData[i].geometryInfo.scratchData.deviceAddress = scratchAddress;
			buildData[i].geometryInfo.dstAccelerationStructure = tempBlas[i].handle;

			// Range info
			std::vector<VkAccelerationStructureBuildRangeInfoKHR*> rangeInfos = {buildData[i].rangeInfos.data()};

			mEngine.mVkbDT.fp_vkCmdBuildAccelerationStructuresKHR(cmd, 1, &buildData[i].geometryInfo, rangeInfos.data());

			VkMemoryBarrier2 barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR;
			barrier.srcStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
			barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
			barrier.dstStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
			barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;

			VkDependencyInfo dependencyInfo{};
			dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
			dependencyInfo.memoryBarrierCount = 1;
			dependencyInfo.pMemoryBarriers = &barrier;

			vkCmdPipelineBarrier2(cmd, &dependencyInfo);

			vkCmdResetQueryPool(cmd, queryPool, i, 1);
			mEngine.mVkbDT.fp_vkCmdWriteAccelerationStructuresPropertiesKHR(
					cmd, 1, &tempBlas[i].handle, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, queryPool, i);
		};

		ImmediateSubmit(mEngine.mDevice, mEngine.mGraphicsQueue, mEngine.mImmediate, func);
	}

	if ((flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR) == VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR)
	{
		std::vector<VkDeviceSize> compactSizes(blasCount);

		vkGetQueryPoolResults(mEngine.mDevice, queryPool, 0, static_cast<u32>(compactSizes.size()),
							  compactSizes.size() * sizeof(VkDeviceSize), compactSizes.data(), sizeof(VkDeviceSize),
							  VK_QUERY_RESULT_WAIT_BIT);

		for (u32 i = 0; i < blasCount; i++)
		{
			auto func = [&](VkCommandBuffer cmd)
			{
				// Create acceleration structure buffer
				VkBufferUsageFlags asBufferUsage{};
				asBufferUsage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
				asBufferUsage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
				output[i].buffer = CreateBuffer(mEngine.mAllocator, compactSizes[i], asBufferUsage, VMA_MEMORY_USAGE_GPU_ONLY);

				// Create info
				VkAccelerationStructureCreateInfoKHR createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
				createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
				createInfo.size = compactSizes[i];
				createInfo.buffer = output[i].buffer.buffer;

				mEngine.mVkbDT.fp_vkCreateAccelerationStructureKHR(mEngine.mDevice, &createInfo, nullptr, &output[i].handle);

				// Copy info
				VkCopyAccelerationStructureInfoKHR copyInfo{};
				copyInfo.sType = VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR;
				copyInfo.src = tempBlas[i].handle;
				copyInfo.dst = output[i].handle;
				copyInfo.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR;

				mEngine.mVkbDT.fp_vkCmdCopyAccelerationStructureKHR(cmd, &copyInfo);
			};

			ImmediateSubmit(mEngine.mDevice, mEngine.mGraphicsQueue, mEngine.mImmediate, func);
		}
	}
	else
	{
		output = tempBlas;
	}

	for (u32 i = 0; i < blasCount; i++)
	{
		VkAccelerationStructureDeviceAddressInfoKHR addressInfo{};
		addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		addressInfo.accelerationStructure = output[i].handle;
		output[i].deviceAddress = mEngine.mVkbDT.fp_vkGetAccelerationStructureDeviceAddressKHR(mEngine.mDevice, &addressInfo);
	}

	for (u32 i = 0; i < blasCount; i++)
	{
		mEngine.mVkbDT.fp_vkDestroyAccelerationStructureKHR(mEngine.mDevice, tempBlas[i].handle, nullptr);
		DestroyBuffer(mEngine.mAllocator, tempBlas[i].buffer);
	}

	vkDestroyQueryPool(mEngine.mDevice, queryPool, nullptr);

	DestroyBuffer(mEngine.mAllocator, scratchBuffer);
}

void RaytracingBuilder::BuildTlas(std::vector<VkAccelerationStructureInstanceKHR>& instances, TLAS& tlas,
								  VkBuildAccelerationStructureFlagsKHR flags)
{
	tlas.instanceCount = static_cast<u32>(instances.size());

	// Create instances buffer
	VkBufferUsageFlags bufferUsage{};
	bufferUsage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	bufferUsage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	bufferUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	// TODO(Sergei): Set up a pipeline barrier instead, don't want to stall the pipeline.
	VkDeviceSize stagingBufferSize = sizeof(VkAccelerationStructureInstanceKHR) * tlas.instanceCount;
	VulkanBuffer stagingBuffer =
			CreateBuffer(mEngine.mAllocator, stagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	memcpy(stagingBuffer.info.pMappedData, instances.data(), stagingBufferSize);

	tlas.instanceBuffer = CreateBuffer(mEngine.mAllocator, stagingBufferSize, bufferUsage, VMA_MEMORY_USAGE_GPU_ONLY);

	auto func1 = [&](VkCommandBuffer cmd)
	{
		CopyBuffer(cmd, stagingBuffer.buffer, tlas.instanceBuffer.buffer, stagingBufferSize);
	};

	ImmediateSubmit(mEngine.mDevice, mEngine.mGraphicsQueue, mEngine.mImmediate, func1);

	DestroyBuffer(mEngine.mAllocator, stagingBuffer);

	// Get device address of instances buffer
	VkBufferDeviceAddressInfo instancesBufferInfo{};
	instancesBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	instancesBufferInfo.buffer = tlas.instanceBuffer.buffer;
	tlas.instanceBufferAddress = vkGetBufferDeviceAddress(mEngine.mDevice, &instancesBufferInfo);

	// Instances data
	VkAccelerationStructureGeometryInstancesDataKHR instancesData{};
	instancesData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	instancesData.data.deviceAddress = tlas.instanceBufferAddress;

	// Geometry
	VkAccelerationStructureGeometryKHR geometry{};
	geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	geometry.geometry.instances = instancesData;

	// Build geometry info
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	buildInfo.flags = flags;
	buildInfo.geometryCount = 1;
	buildInfo.pGeometries = &geometry;

	// Sizes info
	VkAccelerationStructureBuildSizesInfoKHR sizesInfo{};
	sizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	mEngine.mVkbDT.fp_vkGetAccelerationStructureBuildSizesKHR(mEngine.mDevice, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
															  &buildInfo, &tlas.instanceCount, &sizesInfo);

	// Create tlas buffer
	VkBufferUsageFlags asBufferUsage{};
	asBufferUsage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
	asBufferUsage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	tlas.buffer = CreateBuffer(mEngine.mAllocator, sizesInfo.accelerationStructureSize, asBufferUsage, VMA_MEMORY_USAGE_GPU_ONLY);

	// Create info
	VkAccelerationStructureCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	createInfo.size = sizesInfo.accelerationStructureSize;
	createInfo.buffer = tlas.buffer.buffer;
	mEngine.mVkbDT.fp_vkCreateAccelerationStructureKHR(mEngine.mDevice, &createInfo, nullptr, &tlas.handle);

	// Create scratch buffer
	VkBufferUsageFlags scratchBufferUsage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
	tlas.scratchBuffer = CreateBufferAligned(mEngine.mAllocator, sizesInfo.buildScratchSize, scratchBufferUsage, memoryUsage, 128);

	// Get device address of scratch buffer
	VkBufferDeviceAddressInfo scratchBufferInfo{};
	scratchBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	scratchBufferInfo.buffer = tlas.scratchBuffer.buffer;
	tlas.scratchAddress = vkGetBufferDeviceAddress(mEngine.mDevice, &scratchBufferInfo);

	// Build geometry info (cont.)
	buildInfo.dstAccelerationStructure = tlas.handle;
	buildInfo.scratchData.deviceAddress = tlas.scratchAddress;

	// Build range info
	VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
	buildRangeInfo.primitiveCount = tlas.instanceCount;
	const VkAccelerationStructureBuildRangeInfoKHR* pBuildRangeInfo = &buildRangeInfo;

	auto func2 = [&](VkCommandBuffer cmd)
	{
		mEngine.mVkbDT.fp_vkCmdBuildAccelerationStructuresKHR(cmd, 1, &buildInfo, &pBuildRangeInfo);
	};

	ImmediateSubmit(mEngine.mDevice, mEngine.mGraphicsQueue, mEngine.mImmediate, func2);
}

void RaytracingBuilder::UpdateTlas(VkCommandBuffer cmd, TLAS& tlas, VkBuildAccelerationStructureFlagsKHR flags)
{
	assert(tlas.handle != VK_NULL_HANDLE);

	// Instances data
	VkAccelerationStructureGeometryInstancesDataKHR instancesData{};
	instancesData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	instancesData.data.deviceAddress = tlas.instanceBufferAddress;

	// Geometry
	VkAccelerationStructureGeometryKHR geometry{};
	geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	geometry.geometry.instances = instancesData;

	// Build geometry info
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
	buildInfo.flags = flags;
	buildInfo.geometryCount = 1;
	buildInfo.pGeometries = &geometry;
	buildInfo.srcAccelerationStructure = tlas.handle;
	buildInfo.dstAccelerationStructure = tlas.handle;
	buildInfo.scratchData.deviceAddress = tlas.scratchAddress;

	// Build range info
	VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
	buildRangeInfo.primitiveCount = tlas.instanceCount;
	const VkAccelerationStructureBuildRangeInfoKHR* pBuildRangeInfo = &buildRangeInfo;

	mEngine.mVkbDT.fp_vkCmdBuildAccelerationStructuresKHR(cmd, 1, &buildInfo, &pBuildRangeInfo);

	VkMemoryBarrier2 barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR;
	barrier.srcStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
	barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
	barrier.dstStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
	barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;

	VkDependencyInfo dependencyInfo{};
	dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
	dependencyInfo.memoryBarrierCount = 1;
	dependencyInfo.pMemoryBarriers = &barrier;

	vkCmdPipelineBarrier2(cmd, &dependencyInfo);
}
