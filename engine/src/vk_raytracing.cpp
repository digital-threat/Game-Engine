#include <engine.h>
#include <mesh_structs.h>
#include <vertex.h>
#include <vk_buffers.h>
#include <vk_raytracing.h>
#include <vk_utility.h>

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
	vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
		vkGetDeviceProcAddr(mDevice, "vkGetAccelerationStructureBuildSizesKHR"));
	vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(
		mDevice, "vkCmdBuildAccelerationStructuresKHR"));
	vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(
		mDevice, "vkCreateAccelerationStructureKHR"));
}

// TODO(Sergei): Compact BLAS (could save over 50% memory)
// TODO(Sergei): Allocate one big scratch buffer and use regions of it to build different BLAS
// TODO(Sergei): Split workload into batches (otherwise could stall the pipeline and "potentially create problems")
// https://nvpro-samples.github.io/vk_raytracing_tutorial_KHR
void RaytracingBuilder::BuildBlas(std::vector<BlasInput>& input, VkBuildAccelerationStructureFlagsKHR flags)
{
	mBlas.resize(input.size());

	std::vector<VulkanBuffer> scratchBuffers;
	scratchBuffers.reserve(input.size());

	std::vector<AccelerationStructureBuildData> buildData;
	buildData.resize(input.size());

	u32 blasCount = static_cast<u32>(input.size());
	for (u32 i = 0; i < blasCount; i++)
	{
		//Build geometry info
		VkAccelerationStructureBuildGeometryInfoKHR geometryInfo{};
		geometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		geometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		geometryInfo.flags = input[i].flags | flags; // VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		geometryInfo.geometryCount = static_cast<u32>(input[i].asGeometry.size());
		geometryInfo.pGeometries = input[i].asGeometry.data();

		// Build range info
		std::vector<VkAccelerationStructureBuildRangeInfoKHR> rangeInfos = input[i].asBuildOffsetInfo;
		buildData[i].rangeInfos = rangeInfos;

		// Build sizes info
		VkAccelerationStructureBuildSizesInfoKHR sizesInfo{};
		sizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		std::vector<u32> primitiveCounts = input[i].GetPrimitiveCounts();
		auto buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
		vkGetAccelerationStructureBuildSizesKHR(mDevice, buildType, &geometryInfo, primitiveCounts.data(), &sizesInfo);
		buildData[i].sizesInfo = sizesInfo;

		// Create scratch buffer
		VkBufferUsageFlags scratchBufferUsage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		VulkanBuffer scratchBuffer = CreateBuffer(mAllocator, sizesInfo.accelerationStructureSize, scratchBufferUsage, VMA_MEMORY_USAGE_AUTO);
		scratchBuffers.push_back(scratchBuffer);

		// Get device address of scratch buffer
		VkBufferDeviceAddressInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		bufferInfo.buffer = scratchBuffer.buffer;
		VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(mDevice, &bufferInfo);

		// Build info (cont.)
		geometryInfo.scratchData.deviceAddress = scratchAddress;
		geometryInfo.dstAccelerationStructure = mBlas[i].handle;
		buildData[i].geometryInfo = geometryInfo;
	}

	auto func = [&](VkCommandBuffer cmd)
	{
		for (u32 i = 0; i < blasCount; i++)
		{
			// Create acceleration structure buffer
			VkBufferUsageFlags asBufferUsage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
			mBlas[i].buffer = CreateBuffer(mEngine.mAllocator, buildData[i].sizesInfo.accelerationStructureSize, asBufferUsage, VMA_MEMORY_USAGE_AUTO);

			// Create info
			VkAccelerationStructureCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
			createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			createInfo.size = buildData[i].sizesInfo.accelerationStructureSize;
			createInfo.buffer = mBlas[i].buffer.buffer;
			vkCreateAccelerationStructureKHR(mEngine.mDevice, &createInfo, nullptr, &mBlas[i].handle);

			// Range info
			std::vector<VkAccelerationStructureBuildRangeInfoKHR*> rangeInfos = { buildData[i].rangeInfos.data() };

			vkCmdBuildAccelerationStructuresKHR(cmd, 1, &buildData[i].geometryInfo, rangeInfos.data());
		}
	};

	ImmediateSubmit(mEngine.mDevice, mEngine.mGraphicsQueue, mEngine.mImmediate, func);

	for (u32 i = 0; i < scratchBuffers.size(); i++)
	{
		DestroyBuffer(mAllocator, scratchBuffers[i]);
	}
}

void RaytracingBuilder::BuildTlas(std::vector<VkAccelerationStructureInstanceKHR>& instances,
	VkBuildAccelerationStructureFlagsKHR flags)
{
}
