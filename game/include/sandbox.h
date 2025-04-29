#pragma once

#include <acceleration_structures.h>
#include <application.h>
#include <ecs/coordinator.h>
#include <scene.h>

struct RayHit;
struct Ray;

class Sandbox : public Application
{
private:
	Coordinator mGlobalCoordinator;
	bool isSimulating;

	std::vector<Scene> mScenes;
	u32 mCurrentScene;

public:
	explicit Sandbox(Engine& engine);
	void Awake() override;
	void Update(f64 deltaTime) override;
	void PhysicsUpdate(f64 deltaTime) override;
	void Render() override;
	void Destroy() override;

private:
	void ImGuiApplication();
	void ImGuiMaterials();
	void ImGuiMainLight(Scene& scene);

	Scene MirrorScene();
	Scene SponzaScene();
	void CornellBoxScene();

	bool Raycast(Ray& ray, RayHit& hit);
};
