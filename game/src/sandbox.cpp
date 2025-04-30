#include <acceleration_structures.h>
#include <engine.h>
#include <sandbox.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <collision.h>
#include <components/camera.h>
#include <ecs/coordinator.h>
#include <systems/camera_system.h>
#include <systems/rt_render_system.h>
#include <systems/transform_gui_system.h>

Sandbox::Sandbox(Engine& engine) : Application(engine)
{
	mRenderContext.renderScale = 1.0f;
	mRenderContext.samplesPerPixel = 1;
	isSimulating = true;
	mCurrentScene = 0;
}

void Sandbox::Awake()
{
	mGlobalCoordinator.RegisterComponent<Camera>();

	{
		Entity entity = mGlobalCoordinator.CreateEntity();

		Camera camera{};
		camera.position = glm::vec3(0.0f, 2.0f, -3.0f);
		camera.sensitivity = 0.1f;
		camera.speed = 1.0f;
		camera.yaw = 90.0f;
		camera.fov = 60.0f;
		mGlobalCoordinator.AddComponent<Camera>(entity, camera);
	}

	mScenes.push_back(MirrorScene());
	mScenes.push_back(SponzaScene());

	mCurrentScene = 1;
}

void Sandbox::Update(f64 deltaTime)
{
	CameraSystem::Update(mGlobalCoordinator.mEntityManager, mGlobalCoordinator.mComponentManager, mRenderContext.camera, deltaTime);
}

void Sandbox::PhysicsUpdate(f64 deltaTime)
{
	// if (isSimulating)
	// {
	// 	PhysicsSystem::Update(mGlobalCoordinator.mEntityManager, mGlobalCoordinator.mComponentManager, deltaTime);
	// }
}

void Sandbox::Render()
{
	Scene& scene = mScenes[mCurrentScene];

	CameraSystem::OnGUI(mGlobalCoordinator.mEntityManager, mGlobalCoordinator.mComponentManager);
	TransformGUISystem::Update(scene.coordinator.mEntityManager, scene.coordinator.mComponentManager);

	ImGuiApplication();
	ImGuiScene(scene);

	mRenderContext.scene.mainLightPos = scene.mainLightPosition;
	mRenderContext.scene.mainLightColor = glm::vec4(scene.mainLightColor, scene.mainLightIntensity);
	mRenderContext.scene.skyColor = scene.skyColor;
	mRenderContext.instances.clear();
	mRenderContext.light.lightCount = 0;
	mRenderContext.raytracing.tlas = scene.tlas.handle;

	RtRenderSystem::Update(mEngine, scene.coordinator.mEntityManager, scene.coordinator.mComponentManager, scene);
}

void Sandbox::Destroy() {}

// bool Sandbox::Raycast(Ray& ray, RayHit& hit)
// {
// 	auto sphereColliders = mScenes[mCurrentScene].coordinator.mComponentManager.GetComponentsOfType<SphereCollider>();
// 	auto boxColliders = mScenes[mCurrentScene].coordinator.mComponentManager.GetComponentsOfType<BoxCollider>();
//
// 	std::vector<Collision> collisions;
//
// 	for (u32 i = 0; i < sphereColliders.mSize; i++)
// 	{
// 		SphereCollider& c = sphereColliders.mComponentArray[i];
// 		if (IntersectRaySphere(ray, hit, c))
// 		{
// 			return true;
// 		}
// 	}
//
// 	for (u32 i = 0; i < boxColliders.mSize; i++)
// 	{
// 		BoxCollider& c = boxColliders.mComponentArray[i];
// 		if (IntersectRayOBB(ray, hit, c))
// 		{
// 			return true;
// 		}
// 	}
//
// 	return false;
// }
