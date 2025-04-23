#pragma once

#include <acceleration_structures.h>
#include <application.h>
#include <components/camera.h>
#include <ecs/coordinator.h>
#include <systems/resource_system.h>

struct RayHit;
struct Ray;

class Sandbox : public Application
{
private:
	Coordinator mCoordinator;
	ResourceSystem mResourceSystem;
	glm::vec3 mMainLightColor;
	glm::vec3 mMainLightPosition;
	float mMainLightIntensity;
	bool isSimulating;

	RaytracingBuilder rtBuilder;
	Scene rtScene;

public:
	explicit Sandbox(Engine& engine);
	void Awake() override;
	void Update(f64 deltaTime) override;
	void PhysicsUpdate(f64 deltaTime) override;
	void Render() override;
	void Destroy() override;

private:
	void CreateBlas();
	void CreateTlas();

private:
	void ImGuiApplication();
	void ImGuiMaterials();
	void ImGuiMainLight();

	void CreateScene();

	bool Raycast(Ray& ray, RayHit& hit);
};
