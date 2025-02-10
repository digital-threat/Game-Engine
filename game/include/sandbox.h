#pragma once
#include <application.h>
#include <camera.h>
#include <message_queue.h>
#include <entity_manager.h>

struct Ray;

class MySandbox : public Application, public MessageQueue
{
private:
	Camera mCamera{ .position = glm::vec3{ 0.0f, 2.0f, -3.0f }, .fov = 60};
	EntityManager mEntityManager;
	i32 mCurrentEntity = 0;
	glm::vec3 mMainLightColor = glm::vec3(1, 1, 1);
	glm::vec3 mMainLightPosition = glm::vec3(0, 5, -10);
	float mMainLightIntensity = 1.0f;
	bool isSimulating = true;

public:
	void Awake() override;
	void Update() override;
	void PhysicsUpdate() override;
	void Render() override;
	void Destroy() override;

private:
	void ImGuiApplication();
	void ImGuiCamera();
	void ImGuiEntities();
	void ImGuiMaterials();
	void ImGuiMainLight();

	void LoadDefaultScene();

	bool Raycast(Ray& ray);

	void ProcessMessage(Message *pMessage) override;
};

