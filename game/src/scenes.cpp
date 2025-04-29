#include <components/name.h>
#include <components/renderer.h>
#include <components/transform.h>
#include <mesh_manager.h>
#include <sandbox.h>


struct BoxCollider;
struct SphereCollider;

Scene Sandbox::MirrorScene()
{
	Scene scene(mEngine);

	scene.coordinator.RegisterComponent<Transform>();
	scene.coordinator.RegisterComponent<Renderer>();
	scene.coordinator.RegisterComponent<Name>();

	scene.mainLightColor = glm::vec3(1, 1, 1);
	scene.mainLightPosition = glm::vec3(0, 5, -10);
	scene.mainLightIntensity = 0.5f;
	scene.skyColor = glm::vec4(0.27f, 0.69f, 0.86f, 1.0f);

	MeshManager& meshManager = MeshManager::Instance();
	MeshHandle cube = meshManager.LoadMesh("models\\cube\\cube.obj");
	MeshHandle mirror = meshManager.LoadMesh("models\\mirror\\mirror.obj");
	// MeshHandle plane = meshManager.LoadMesh("models\\plane\\plane.obj");
	// MeshHandle sphere = meshManager.LoadMesh("models\\sphere\\sphere.obj");
	scene.meshes.push_back(cube);
	scene.meshes.push_back(mirror);
	// scene.meshes.push_back(plane);

	{
		Entity entity = scene.coordinator.CreateEntity();

		Name name;
		name.name = "Cube";
		scene.coordinator.AddComponent<Name>(entity, name);

		Transform transform;
		transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
		transform.rotation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
		transform.scale = 1;
		scene.coordinator.AddComponent<Transform>(entity, transform);

		Renderer renderer;
		renderer.meshHandle = cube;
		scene.coordinator.AddComponent<Renderer>(entity, renderer);
	}

	// {
	// 	Entity entity = scene.coordinator.CreateEntity();
	//
	// 	Name name;
	// 	name.name = "Plane";
	// 	scene.coordinator.AddComponent<Name>(entity, name);
	//
	// 	Transform transform;
	// 	transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
	// 	transform.rotation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
	// 	transform.scale = 5.0f;
	// 	scene.coordinator.AddComponent<Transform>(entity, transform);
	//
	// 	Renderer renderer;
	// 	renderer.meshHandle = plane;
	// 	scene.coordinator.AddComponent<Renderer>(entity, renderer);
	// }

	{
		Entity entity = scene.coordinator.CreateEntity();

		Name name;
		name.name = "Mirror";
		scene.coordinator.AddComponent<Name>(entity, name);

		Transform transform;
		transform.position = glm::vec3(0.0f, 2.0f, 0.0f);
		transform.rotation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
		transform.scale = 5.0f;
		scene.coordinator.AddComponent<Transform>(entity, transform);

		Renderer renderer;
		renderer.meshHandle = mirror;
		scene.coordinator.AddComponent<Renderer>(entity, renderer);
	}

	{
		Entity entity = scene.coordinator.CreateEntity();

		Name name;
		name.name = "Mirror";
		scene.coordinator.AddComponent<Name>(entity, name);

		Transform transform;
		transform.position = glm::vec3(0.0f, -2.0f, 0.0f);
		transform.rotation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
		transform.scale = 5.0f;
		scene.coordinator.AddComponent<Transform>(entity, transform);

		Renderer renderer;
		renderer.meshHandle = mirror;
		scene.coordinator.AddComponent<Renderer>(entity, renderer);
	}

	// {
	// 	Entity entity = scene.coordinator.CreateEntity();
	//
	// 	Name name;
	// 	name.name = "Sphere";
	// 	scene.coordinator.AddComponent<Name>(entity, name);
	//
	// 	Transform transform;
	// 	transform.position = glm::vec3(-1.5f, 0.6f, -1.5f);
	// 	transform.rotation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
	// 	transform.scale = 0.5f;
	// 	scene.coordinator.AddComponent<Transform>(entity, transform);
	//
	// 	Renderer renderer;
	// 	renderer.meshHandle = sphere;
	// 	scene.coordinator.AddComponent<Renderer>(entity, renderer);
	// }

	scene.CreateBlas();
	scene.CreateTlas();

	return scene;
}

Scene Sandbox::SponzaScene()
{
	Scene scene(mEngine);

	scene.coordinator.RegisterComponent<Transform>();
	scene.coordinator.RegisterComponent<Renderer>();
	scene.coordinator.RegisterComponent<Name>();

	scene.mainLightColor = glm::vec3(1, 1, 1);
	scene.mainLightPosition = glm::vec3(0, 5, -10);
	scene.mainLightIntensity = 0.5f;
	scene.skyColor = glm::vec4(0.27f, 0.69f, 0.86f, 1.0f);

	MeshManager& meshManager = MeshManager::Instance();
	MeshHandle sponza = meshManager.LoadMesh("models\\crytek_sponza\\sponza.obj");
	scene.meshes.push_back(sponza);

	{
		Entity entity = scene.coordinator.CreateEntity();

		Name name;
		name.name = "Sponza";
		scene.coordinator.AddComponent<Name>(entity, name);

		Transform transform;
		transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
		transform.rotation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
		transform.scale = 1;
		scene.coordinator.AddComponent<Transform>(entity, transform);

		Renderer renderer;
		renderer.meshHandle = sponza;
		scene.coordinator.AddComponent<Renderer>(entity, renderer);
	}

	scene.CreateBlas();
	scene.CreateTlas();

	return scene;
}
