#pragma once

#include <glm/vec3.hpp>
#include <component.h>
#include <handles.h>
#include <message_queue.h>

enum class ComponentType
{
	TRANSFORM,
	MESH,
	LIGHT
};

class TransformComponent : public Component
{
public:
	glm::vec3 mPosition = glm::vec3();
	glm::vec3 mRotation = glm::vec3();
	float mScale = 1;

	void Update() override;
	void Render(RenderContext& context, ModelRenderData &renderData) override;
	void OnGUI() override;
};

class MeshComponent : public Component, public MessageQueue
{
public:
	MeshComponent();
	MeshAsset* mMesh = nullptr;
	MaterialHandle mMaterial = MaterialHandle();

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

	void Update() override;
	void Render(RenderContext& context, ModelRenderData &renderData) override;
	void OnGUI() override;
};