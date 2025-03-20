#pragma once

#include <application.h>
#include <components/camera.h>
#include <message_queue.h>
#include <vk_raytracing.h>
#include <ecs/coordinator.h>
#include <systems/resource_system.h>

struct RayHit;
struct Ray;

class Sandbox : public Application, public MessageQueue
{
private:
	Coordinator mCoordinator{};
	glm::vec3 mMainLightColor = glm::vec3(1, 1, 1);
	glm::vec3 mMainLightPosition = glm::vec3(0, 5, -10);
	float mMainLightIntensity = 1.0f;
	bool isSimulating = true;
	ResourceSystem mResourceSystem;

	RaytracingBuilder mRtBuilder;

public:
	Sandbox();
	void Awake() override;
	void Update(f64 deltaTime) override;
	void PhysicsUpdate(f64 deltaTime) override;
	void Render() override;
	void Destroy() override;

private:
	void CreateBlas();

private:
	void ImGuiApplication();
	//void ImGuiEntities();
	void ImGuiMaterials();
	void ImGuiMainLight();

	void LoadDefaultScene();

	bool Raycast(Ray& ray, RayHit& hit);

	void ProcessMessage(Message *pMessage) override;
};

