#include <engine.h>
#include <mesh_structs.h>
#include <utility.h>
#include <vertex.h>
#include <vk_buffers.h>
#include <vk_raytracing.h>
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

	BlasInput input{};
	input.asGeometry.emplace_back(asGeometry);
	input.asBuildOffsetInfo.emplace_back(offset);
	return input;
}

RaytracingBuilder::RaytracingBuilder(Engine& engine): mEngine(engine)
{
	mTlas.handle = VK_NULL_HANDLE;
	//vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(mEngine.mDevice, "vkGetAccelerationStructureBuildSizesKHR"));
	//vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(mEngine.mDevice, "vkCmdBuildAccelerationStructuresKHR"));
	//vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(mEngine.mDevice, "vkCreateAccelerationStructureKHR"));
	//vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(mEngine.mDevice, "vkGetAccelerationStructureDeviceAddressKHR"));
}

// TODO(Sergei): Compact BLAS (could save over 50% memory)
// TODO(Sergei): Allocate one big scratch buffer and use regions of it to build different BLAS
// TODO(Sergei): Split workload into batches (otherwise could stall the pipeline and "potentially create problems")
// https://nvpro-samples.github.io/vk_raytracing_tutorial_KHR
void RaytracingBuilder::BuildBlas(std::vector<BlasInput>& input, VkBuildAccelerationStructureFlagsKHR flags)
{
	mBlas.resize(input.size());

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
		mEngine.mVkbDispatchTable.fp_vkGetAccelerationStructureBuildSizesKHR(mEngine.mDevice, buildType, &geometryInfo, primitiveCounts.data(), &sizesInfo);
		buildData[i].sizesInfo = sizesInfo;

		maxScratchSize = std::max(maxScratchSize, sizesInfo.buildScratchSize);
	}

	// Create scratch buffer
	VkBufferUsageFlags scratchBufferUsage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	VkDeviceSize scratchSize = AlignUp(maxScratchSize, 128);
	VulkanBuffer scratchBuffer = CreateBuffer(mEngine.mAllocator, scratchSize, scratchBufferUsage, VMA_MEMORY_USAGE_GPU_ONLY);

	// Get device address of scratch buffer
	VkBufferDeviceAddressInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	bufferInfo.buffer = scratchBuffer.buffer;
	VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(mEngine.mDevice, &bufferInfo);
	scratchAddress = AlignUp(scratchAddress, 128);

	auto func = [&](VkCommandBuffer cmd)
	{
		for (u32 i = 0; i < blasCount; i++)
		{
			// Create acceleration structure buffer
			VkBufferUsageFlags asBufferUsage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
			mBlas[i].buffer = CreateBuffer(mEngine.mAllocator, buildData[i].sizesInfo.accelerationStructureSize, asBufferUsage, VMA_MEMORY_USAGE_GPU_ONLY);

			// Create info
			VkAccelerationStructureCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
			createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			createInfo.size = buildData[i].sizesInfo.accelerationStructureSize;
			createInfo.buffer = mBlas[i].buffer.buffer;
			mEngine.mVkbDispatchTable.fp_vkCreateAccelerationStructureKHR(mEngine.mDevice, &createInfo, nullptr, &mBlas[i].handle);

			// Build geometry info (cont.)
			buildData[i].geometryInfo.scratchData.deviceAddress = scratchAddress;
			buildData[i].geometryInfo.dstAccelerationStructure = mBlas[i].handle;

			// Range info
			std::vector<VkAccelerationStructureBuildRangeInfoKHR*> rangeInfos = { buildData[i].rangeInfos.data() };

			mEngine.mVkbDispatchTable.fp_vkCmdBuildAccelerationStructuresKHR(cmd, 1, &buildData[i].geometryInfo, rangeInfos.data());
		}
	};

	ImmediateSubmit(mEngine.mDevice, mEngine.mGraphicsQueue, mEngine.mImmediate, func);

	for (u32 i = 0; i < blasCount; i++)
	{
		VkAccelerationStructureDeviceAddressInfoKHR addressInfo{};
		addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		addressInfo.accelerationStructure = mBlas[i].handle;
		mBlas[i].deviceAddress = mEngine.mVkbDispatchTable.fp_vkGetAccelerationStructureDeviceAddressKHR(mEngine.mDevice, &addressInfo);
	}

	DestroyBuffer(mEngine.mAllocator, scratchBuffer);
}

void RaytracingBuilder::BuildTlas(std::vector<VkAccelerationStructureInstanceKHR>& instances, VkBuildAccelerationStructureFlagsKHR flags, bool update)
{
	assert(mTlas.handle == VK_NULL_HANDLE || update);

	u32 instanceCount = static_cast<u32>(instances.size());

	// Create instances buffer
	VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	// TODO(Sergei): Implicit immediate submit. Set up a pipeline barrier instead, don't want to stall the pipeline just to update TLAS.
	VulkanBuffer instancesBuffer = CreateBufferAndUploadData(mEngine, instances, bufferUsage);

	// Get device address of instances buffer
	VkBufferDeviceAddressInfo instancesBufferInfo{};
	instancesBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	instancesBufferInfo.buffer = instancesBuffer.buffer;
	VkDeviceAddress instancesBufferAddress = vkGetBufferDeviceAddress(mEngine.mDevice, &instancesBufferInfo);

	// Instances data
	VkAccelerationStructureGeometryInstancesDataKHR instancesData{};
	instancesData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	instancesData.data.deviceAddress = instancesBufferAddress;

	// Geometry
	VkAccelerationStructureGeometryKHR geometry{};
	geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	geometry.geometry.instances = instancesData;

	// Build geometry info
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	buildInfo.mode = update ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	buildInfo.flags = flags;
	buildInfo.geometryCount = 1;
	buildInfo.pGeometries = &geometry;

	//Sizes info
	VkAccelerationStructureBuildSizesInfoKHR sizesInfo{};
	sizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	mEngine.mVkbDispatchTable.fp_vkGetAccelerationStructureBuildSizesKHR(mEngine.mDevice, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &instanceCount, &sizesInfo);

	// Create acceleration structure buffer
	VkBufferUsageFlags asBufferUsage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	mTlas.buffer = CreateBuffer(mEngine.mAllocator, sizesInfo.accelerationStructureSize, asBufferUsage, VMA_MEMORY_USAGE_GPU_ONLY);

	// Create info
	VkAccelerationStructureCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	createInfo.size = sizesInfo.accelerationStructureSize;
	createInfo.buffer = mTlas.buffer.buffer;
	mEngine.mVkbDispatchTable.fp_vkCreateAccelerationStructureKHR(mEngine.mDevice, &createInfo, nullptr, &mTlas.handle);

	// Create scratch buffer
	VkBufferUsageFlags scratchBufferUsage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	VkDeviceSize scratchSize = AlignUp(sizesInfo.buildScratchSize, 128);
	VulkanBuffer scratchBuffer = CreateBuffer(mEngine.mAllocator, scratchSize, scratchBufferUsage, VMA_MEMORY_USAGE_GPU_ONLY);

	// Get device address of scratch buffer
	VkBufferDeviceAddressInfo scratchBufferInfo{};
	scratchBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	scratchBufferInfo.buffer = scratchBuffer.buffer;
	VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(mEngine.mDevice, &scratchBufferInfo);
	scratchAddress = AlignUp(scratchAddress, 128);

	// Build geometry info (cont.)
	buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;
	buildInfo.dstAccelerationStructure = mTlas.handle;
	buildInfo.scratchData.deviceAddress = scratchAddress;

	// Build range info
	VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
	buildRangeInfo.primitiveCount = instanceCount;
	const VkAccelerationStructureBuildRangeInfoKHR* pBuildRangeInfo = &buildRangeInfo;

	auto func = [&](VkCommandBuffer cmd)
	{
		mEngine.mVkbDispatchTable.fp_vkCmdBuildAccelerationStructuresKHR(cmd, 1, &buildInfo, &pBuildRangeInfo);
	};

	ImmediateSubmit(mEngine.mDevice, mEngine.mGraphicsQueue, mEngine.mImmediate, func);

	// Get device address of TLAS
	VkAccelerationStructureDeviceAddressInfoKHR addressInfo{};
	addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	addressInfo.accelerationStructure = mTlas.handle;
	mEngine.mVkbDispatchTable.fp_vkGetAccelerationStructureDeviceAddressKHR(mEngine.mDevice, &addressInfo);

	DestroyBuffer(mEngine.mAllocator, instancesBuffer);
	DestroyBuffer(mEngine.mAllocator, scratchBuffer);
}

VkDeviceAddress RaytracingBuilder::GetBlasDeviceAddress(u32 index)
{
	assert(index < mBlas.size());
	return mBlas[index].deviceAddress;
}
