#include <acceleration_structures.h>
#include <components/renderer.h>
#include <components/transform.h>
#include <ecs/component_manager.h>
#include <ecs/entity_manager.h>
#include <ecs/typedefs.h>
#include <engine.h>
#include <scene.h>
#include <systems/render_system.h>
#include <vk_buffers.h>
#include <vk_immediate.h>

void RenderSystem::Update(VkCommandBuffer cmd, FrameData& currentFrame, Engine& engine, Scene& scene)
{
	std::vector<VkAccelerationStructureInstanceKHR> instances;
	instances.reserve(scene.tlas.instanceCount);

	Archetype archetype;
	archetype.set(scene.coordinator.mComponentManager.GetComponentType<Transform>());
	archetype.set(scene.coordinator.mComponentManager.GetComponentType<Renderer>());

	auto func1 = [&](Entity entity)
	{
		Transform& transform = scene.coordinator.mComponentManager.GetComponent<Transform>(entity);
		Renderer& renderer = scene.coordinator.mComponentManager.GetComponent<Renderer>(entity);

		glm::mat4 matrixM = glm::translate(glm::mat4(1.0f), transform.position);
		matrixM *= glm::mat4_cast(glm::quat(glm::radians(transform.rotation)));
		matrixM = glm::scale(matrixM, glm::vec3(transform.scale));

		VkAccelerationStructureInstanceKHR instance{};
		instance.transform = ToVkTransformMatrixKHR(matrixM);
		instance.instanceCustomIndex = renderer.meshHandle;
		instance.accelerationStructureReference = scene.GetBlasDeviceAddress(renderer.meshHandle);
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instances.emplace_back(instance);
	};

	scene.coordinator.mEntityManager.Each(archetype, func1);

	VkDeviceSize stagingBufferSize = sizeof(VkAccelerationStructureInstanceKHR) * scene.tlas.instanceCount;
	VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	VulkanBuffer stagingBuffer = CreateBuffer(engine.mAllocator, stagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, memoryUsage);
	memcpy(stagingBuffer.info.pMappedData, instances.data(), stagingBufferSize);
	CopyBuffer(cmd, stagingBuffer.buffer, scene.tlas.instanceBuffer.buffer, stagingBufferSize);

	auto func2 = [=]
	{
		DestroyBuffer(engine.mAllocator, stagingBuffer);
	};

	currentFrame.deletionQueue.Push(func2);

	VkMemoryBarrier2 barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR;
	barrier.srcStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
	barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR,
	barrier.dstStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
	barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;

	VkDependencyInfo dependencyInfo{};
	dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
	dependencyInfo.memoryBarrierCount = 1;
	dependencyInfo.pMemoryBarriers = &barrier;

	vkCmdPipelineBarrier2(cmd, &dependencyInfo);

	VkBuildAccelerationStructureFlagsKHR flags{};
	flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
	scene.rtBuilder.UpdateTlas(cmd, scene.tlas, flags);
}
