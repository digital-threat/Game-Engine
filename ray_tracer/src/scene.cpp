#include <components/renderer.h>
#include <components/transform.h>
#include <mesh_manager.h>
#include <scene.h>

// TODO(Sergei): I should probably have a globally accessible instance of Engine
Scene::Scene(Engine& engine) : rtBuilder(engine) {}

VkDeviceAddress Scene::GetBlasDeviceAddress(MeshHandle handle)
{
	assert(blasMap.contains(handle));
	return blasMap[handle].deviceAddress;
}

void Scene::CreateBlas()
{
	MeshManager& meshManager = MeshManager::Instance();
	u32 size = meshes.size();

	std::vector<BlasInput> input;
	input.reserve(size);

	for (u32 i = 0; i < size; i++)
	{
		BlasInput blas = MeshToVkGeometryKHR(meshManager.mMeshes[meshes[i]]);
		input.push_back(blas);
	}
	VkBuildAccelerationStructureFlagsKHR flags{};
	flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;

	std::vector<BLAS> blas;
	rtBuilder.BuildBlas(input, blas, flags);

	for (u32 i = 0; i < size; i++)
	{
		blasMap[meshes[i]] = blas[i];
	}
}
void Scene::CreateTlas()
{
	std::vector<VkAccelerationStructureInstanceKHR> instances;

	Archetype archetype;
	archetype.set(coordinator.mComponentManager.GetComponentType<Transform>());
	archetype.set(coordinator.mComponentManager.GetComponentType<Renderer>());

	auto func = [&](Entity entity)
	{
		Transform& transform = coordinator.mComponentManager.GetComponent<Transform>(entity);
		Renderer& renderer = coordinator.mComponentManager.GetComponent<Renderer>(entity);

		glm::mat4 matrixM = glm::translate(glm::mat4(1.0f), transform.position);
		matrixM *= glm::mat4_cast(glm::quat(glm::radians(transform.rotation)));
		matrixM = glm::scale(matrixM, glm::vec3(transform.scale));

		VkAccelerationStructureInstanceKHR instance{};
		instance.transform = ToVkTransformMatrixKHR(matrixM);
		instance.instanceCustomIndex = renderer.meshHandle;
		instance.accelerationStructureReference = GetBlasDeviceAddress(renderer.meshHandle);
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instances.emplace_back(instance);
	};

	coordinator.mEntityManager.Each(archetype, func);

	VkBuildAccelerationStructureFlagsKHR flags{};
	flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
	rtBuilder.BuildTlas(instances, tlas, flags);
}
