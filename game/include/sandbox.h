#pragma once
#include <application.h>
#include <camera.h>
#include <message_queue.h>
#include <entity_manager.h>

class MySandbox : public Application, public MessageQueue
{
private:
	Camera mCamera{ .position = glm::vec3{ 0.0f, 2.0f, -3.0f }, .fov = 60};
	EntityManager mEntityManager;
	int mCurrentEntity = 0;

public:
	void Awake() override;
	void Update() override;
	void Render() override;
	void Destroy() override;

private:
	void ImGuiApplication();
	void ImGuiCamera();
	void ImGuiEntities();
	void ImGuiMaterials();

	void LoadDefaultScene();

	void ProcessMessage(Message *pMessage) override;
};

