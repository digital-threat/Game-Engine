#pragma once

#include <glm/vec3.hpp>
#include <component.h>
#include <handles.h>
#include <message_queue.h>


class TransformComponent : public Component
{
public:
	glm::vec3 mPosition;
	glm::vec3 mRotation;
	float mScale = 1;

	void Update() override;
	void Render(ModelRenderData &renderData) override;
	void OnGUI() override;
};

class MeshComponent : public Component, public MessageQueue
{
public:
	MeshAsset* mMesh = nullptr;
	MaterialHandle mMaterial = MaterialHandle();

	void Update() override;
	void Render(ModelRenderData &renderData) override;
	void OnGUI() override;
	void ProcessMessage(Message *pMessage) override;
};