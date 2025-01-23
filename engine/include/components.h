#pragma once

#include <glm/vec3.hpp>
#include <component.h>
#include <handles.h>
#include <message_queue.h>

class TransformComponent : public Component
{
public:
	glm::vec3 mPosition = glm::vec3();
	glm::vec3 mRotation = glm::vec3();
	float mScale = 1;

	TransformComponent(Entity &parent) : Component(parent) { mComponentType = ComponentType::TRANSFORM; }

	void Update() override;
	void Render(RenderContext& context, ModelRenderData &renderData) override;
	void OnGUI() override;
};

class MeshComponent : public Component, public MessageQueue
{
public:
	MeshAsset* mMesh = nullptr;
	MaterialHandle mMaterial = MaterialHandle();

	MeshComponent(Entity &parent) : Component(parent) { mComponentType = ComponentType::MESH; }

	void Update() override;
	void Render(RenderContext& context, ModelRenderData &renderData) override;
	void OnGUI() override;
	void ProcessMessage(Message *pMessage) override;
};

class LightComponent : public Component
{
public:
	LightType mType;
	glm::vec3 mColor;
	float mRange;
	float mAngle;

	LightComponent(Entity &parent) : Component(parent) { mComponentType = ComponentType::LIGHT; }

	void Update() override;
	void Render(RenderContext& context, ModelRenderData &renderData) override;
	void OnGUI() override;
};