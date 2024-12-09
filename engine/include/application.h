#pragma once
#include <camera.h>
#include <entity_manager.h>

struct Application
{
	Camera mCamera{ .position = glm::vec3{ 0.0f, 2.0f, -3.0f }, .fov = 60};

	EntityManager mEntityManager;
	int mCurrentEntity = 0;

	float mRenderScale = 0.9f;

	virtual void Awake() = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;
	virtual void Destroy() = 0;
};
