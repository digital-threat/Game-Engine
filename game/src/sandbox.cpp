#include <engine.h>
#include <sandbox.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <collision.h>
#include <components/camera.h>
#include <components/renderer.h>
#include <components/transform.h>
#include <ecs/coordinator.h>
#include <mesh_manager.h>
#include <mesh_structs.h>
#include <obj_loading.h>
#include <systems/camera_system.h>
#include <systems/physics_system.h>
#include <systems/render_system.h>
#include <vk_images.h>
#include <vk_raytracing.h>

Sandbox::Sandbox(Engine& engine) : Application(engine), mResourceSystem(mCoordinator), mRtBuilder(engine)
{
	mRenderContext.renderScale = 1.0f;
	mMainLightColor = glm::vec3(1, 1, 1);
	mMainLightPosition = glm::vec3(0, 5, -10);
	mMainLightIntensity = 0.5f;
	isSimulating = true;
}

void Sandbox::Awake()
{
	mCoordinator.RegisterComponent<Transform>();
	mCoordinator.RegisterComponent<Renderer>();
	mCoordinator.RegisterComponent<SphereCollider>();
	mCoordinator.RegisterComponent<BoxCollider>();
	mCoordinator.RegisterComponent<Camera>();

	CreateScene();

	CreateBlas();
	CreateTlas();
}

void Sandbox::Update(f64 deltaTime)
{
	mResourceSystem.Update();

	CameraSystem::Update(mCoordinator.mEntityManager, mCoordinator.mComponentManager, mRenderContext.camera, deltaTime);

	Ray ray{};
	RayHit hit{};
	ray.direction = glm::vec3(0, 0, 1);
	Raycast(ray, hit);
}

void Sandbox::PhysicsUpdate(f64 deltaTime)
{
	if (isSimulating)
	{
		PhysicsSystem::Update(mCoordinator.mEntityManager, mCoordinator.mComponentManager, deltaTime);
	}
}

void Sandbox::Render()
{
	CameraSystem::OnGUI(mCoordinator.mEntityManager, mCoordinator.mComponentManager);

	ImGuiApplication();
	// ImGuiEntities();
	// ImGuiMaterials();
	ImGuiMainLight();

	mRenderContext.scene.ambientColor = glm::vec3(0.05f, 0.05f, 0.05f);
	mRenderContext.scene.mainLightPos = mMainLightPosition;
	mRenderContext.scene.mainLightColor = glm::vec4(mMainLightColor, mMainLightIntensity);
	mRenderContext.instances.clear();
	mRenderContext.light.lightCount = 0;
	mRenderContext.raytracing.tlas = mRtBuilder.mTlas.handle;

	RenderSystem::Update(mCoordinator.mEntityManager, mCoordinator.mComponentManager, mRenderContext);
}

void Sandbox::Destroy() {}

void Sandbox::CreateBlas()
{
	MeshManager& meshManager = MeshManager::Instance();
	u32 size = meshManager.mMeshes.size();

	std::vector<BlasInput> input;
	input.reserve(size);

	for (u32 i = 0; i < size; i++)
	{
		BlasInput blas = MeshToVkGeometryKHR(meshManager.mMeshes[i]);
		input.emplace_back(blas);
	}

	mRtBuilder.BuildBlas(input, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
}

void Sandbox::CreateTlas()
{
	std::vector<VkAccelerationStructureInstanceKHR> tlas;

	Archetype archetype;
	archetype.set(mCoordinator.mComponentManager.GetComponentType<Transform>());
	archetype.set(mCoordinator.mComponentManager.GetComponentType<Renderer>());

	auto func = [&](Entity entity)
	{
		Transform& transform = mCoordinator.mComponentManager.GetComponent<Transform>(entity);
		Renderer& renderer = mCoordinator.mComponentManager.GetComponent<Renderer>(entity);

		glm::mat4 matrixM = glm::translate(glm::mat4(1.0f), transform.position);
		matrixM *= glm::mat4_cast(transform.rotation);
		matrixM = glm::scale(matrixM, glm::vec3(transform.scale));

		VkAccelerationStructureInstanceKHR instance{};
		instance.transform = ToVkTransformMatrixKHR(matrixM);
		instance.instanceCustomIndex = renderer.meshHandle;
		instance.accelerationStructureReference = mRtBuilder.GetBlasDeviceAddress(renderer.meshHandle);
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		tlas.emplace_back(instance);
	};

	mCoordinator.mEntityManager.Each(archetype, func);

	mRtBuilder.BuildTlas(tlas, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
}

void Sandbox::CreateScene()
{
	MeshManager& meshManager = MeshManager::Instance();
	MeshHandle cube = meshManager.LoadMesh("assets\\meshes\\cube.obj");
	MeshHandle plane = meshManager.LoadMesh("assets\\meshes\\plane.obj");

	{
		Entity entity = mCoordinator.CreateEntity();

		Transform transform;
		transform.position = glm::vec3(0.5f, 0.6f, 0.5f);
		transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		transform.scale = 1;
		mCoordinator.AddComponent<Transform>(entity, transform);

		Renderer renderer;
		renderer.meshHandle = cube;
		mCoordinator.AddComponent<Renderer>(entity, renderer);
	}

	{
		Entity entity = mCoordinator.CreateEntity();

		Transform transform;
		transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
		transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		transform.scale = 0.25f;
		mCoordinator.AddComponent<Transform>(entity, transform);

		Renderer renderer;
		renderer.meshHandle = plane;
		mCoordinator.AddComponent<Renderer>(entity, renderer);
	}

	{
		Entity entity = mCoordinator.CreateEntity();

		Camera camera{};
		camera.position = glm::vec3(0.0f, 2.0f, -3.0f);
		camera.sensitivity = 0.1f;
		camera.speed = 1.0f;
		camera.yaw = 90.0f;
		camera.fov = 60.0f;
		mCoordinator.AddComponent<Camera>(entity, camera);
	}
}

bool Sandbox::Raycast(Ray& ray, RayHit& hit)
{
	auto sphereColliders = mCoordinator.mComponentManager.GetComponentsOfType<SphereCollider>();
	auto boxColliders = mCoordinator.mComponentManager.GetComponentsOfType<BoxCollider>();

	std::vector<Collision> collisions;

	for (u32 i = 0; i < sphereColliders.mSize; i++)
	{
		SphereCollider& c = sphereColliders.mComponentArray[i];
		if (IntersectRaySphere(ray, hit, c))
		{
			return true;
		}
	}

	for (u32 i = 0; i < boxColliders.mSize; i++)
	{
		BoxCollider& c = boxColliders.mComponentArray[i];
		if (IntersectRayOBB(ray, hit, c))
		{
			return true;
		}
	}

	return false;
}
