#pragma once

#include <application.h>
#include <components/camera.h>
#include <message_queue.h>
#include <vk_raytracing.h>
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

	RaytracingBuilder mRtBuilder;

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
	//void ImGuiEntities();
	void ImGuiMaterials();
	void ImGuiMainLight();

	void LoadDefaultScene();

	bool Raycast(Ray& ray, RayHit& hit);
};

