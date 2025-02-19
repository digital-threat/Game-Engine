#pragma once

#include <collision.h>
#include <glm/vec3.hpp>
#include <component.h>
#include <handles.h>
#include <message_queue.h>

enum class ColliderType;

enum class ComponentType
{
	TRANSFORM,
	MESH,
	LIGHT,
	SPHERE_COLLIDER,
	BOX_COLLIDER,
};

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
	float mAngle;

	LightComponent(Entity &parent) : Component(parent) { mComponentType = ComponentType::LIGHT; }

	void Update() override;
	void Render(RenderContext& context, ModelRenderData &renderData) override;
	void OnGUI() override;
};

class ColliderComponent : public Component
{
public:
	ColliderType mType;

	ColliderComponent(Entity &parent);

	virtual Collider& GetCollider() = 0;
};

class SphereColliderComponent : public ColliderComponent
{
public:
	SphereCollider mCollider;
	bool hasGravity = false;
	bool isKinematic = true;

	SphereColliderComponent(Entity &parent);

	void Update() override;
	void Render(RenderContext& context, ModelRenderData &renderData) override;
	void OnGUI() override;

	Collider& GetCollider() override;
};

class BoxColliderComponent : public ColliderComponent
{
public:
	BoxCollider mCollider;

	BoxColliderComponent(Entity &parent);

	void Update() override;
	void Render(RenderContext& context, ModelRenderData &renderData) override;
	void OnGUI() override;

	Collider& GetCollider() override;
};

