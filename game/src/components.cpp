#include "components.h"

#include <imgui.h>
#include <material_manager.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <collision.h>
#include <entity.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <filesystem>
#include <iostream>
#include <mesh_manager.h>
#include <ranges>

void TransformComponent::Update()
{
}

void TransformComponent::Render(RenderContext& context, ModelRenderData &renderData)
{
	glm::mat4 matrixM = glm::translate(glm::mat4(1.0f), mPosition);
	glm::quat rotation = glm::quat(glm::radians(mRotation));
	matrixM *= glm::toMat4(rotation);
	matrixM = glm::scale(matrixM, glm::vec3(mScale));

	renderData.transform = matrixM;
}

void TransformComponent::OnGUI()
{
	ImGui::InputFloat3("Position", reinterpret_cast<float *>(&mPosition));
	ImGui::InputFloat3("Rotation", reinterpret_cast<float *>(&mRotation));
	ImGui::InputFloat("Scale", &mScale);
}

void MeshComponent::Update()
{
	ProcessMessages();
}

void MeshComponent::Render(RenderContext& context, ModelRenderData &renderData)
{
	if (mMesh != nullptr)
	{
		renderData.indexBuffer = mMesh->meshBuffers.indexBuffer;
		renderData.indexCount = mMesh->indexCount;
		renderData.vertexBuffer = mMesh->meshBuffers.vertexBuffer;
		renderData.vertexBufferAddress = mMesh->meshBuffers.vertexBufferAddress;
	}
	if (mMaterial.index != -1)
	{
		renderData.materialSet = MaterialManager::Get().GetDescriptorSet(mMaterial);
	}
}

void MeshComponent::OnGUI()
{
	if (ImGui::Button("Select Mesh"))
	{
		ImGui::OpenPopup("Mesh Selector");
	}

	if (ImGui::BeginPopup("Mesh Selector"))
	{
		ImGui::Text("MESHES:");
		ImGui::Separator();

		for (const auto& file : std::filesystem::directory_iterator("assets/meshes/"))
		{
			if (file.path().extension() == ".bin")
			{
				if (ImGui::Selectable(file.path().filename().string().c_str()))
				{
					MeshManager& meshManager = MeshManager::Get();
					StringMessage* message = new StringMessage("LoadMesh", file.path().string().c_str(), this);
					meshManager.QueueMessage(message);
					ImGui::CloseCurrentPopup();
				}
			}
		}

		ImGui::EndPopup();
	}

	if (ImGui::Button("Select Material"))
	{
		ImGui::OpenPopup("Material Selector");
	}

	if (ImGui::BeginPopup("Material Selector"))
	{
		ImGui::Text("MATERIALS:");
		ImGui::Separator();

		for (const Material& material : MaterialManager::Get().GetAll())
		{
			if (ImGui::Selectable(material.name.c_str()))
			{
				mMaterial = material.handle;
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndPopup();
	}
}

void MeshComponent::ProcessMessage(Message *pMessage)
{
	std::string& message = pMessage->message;
	switch(pMessage->type)
	{
		case MessageType::MESH:
		{
			if (message == "MeshLoaded")
			{
				auto meshMessage = static_cast<MeshMessage *>(pMessage);

				mMesh = new MeshAsset(meshMessage->param);
			}
		} break;
	}
}

void LightComponent::Update()
{
}

void LightComponent::Render(RenderContext& context, ModelRenderData &renderData)
{
	if (context.lightData.lightCount < MAX_LIGHTS - 1)
	{
		TransformComponent* transform = static_cast<TransformComponent *>(parent.GetComponent(ComponentType::TRANSFORM));
		if (transform != nullptr)
		{
			LightData data{};

			switch (mType)
			{
				case LightType::DIRECTIONAL:
				{
					data.color = glm::vec4(mColor, 0.0f);
					glm::quat rotation = glm::quat(glm::radians(transform->mRotation));
					glm::vec3 direction = rotation * glm::vec3(0.0f, 0.0f, 1.0f);
					direction = glm::normalize(direction);
					data.position =glm::vec4(direction, 0.0f);
				} break;
				case LightType::POINT:
				{
					data.color = glm::vec4(mColor, 1.0f);
					data.position = glm::vec4(transform->mPosition, 0.0f);
				} break;
				case LightType::SPOT:
				{
					data.color = glm::vec4(mColor, 2.0f);
					data.position = glm::vec4(transform->mPosition, 0.0f);
					glm::quat rotation = glm::quat(glm::radians(transform->mRotation));
					glm::vec3 direction = rotation * glm::vec3(0.0f, 0.0f, 1.0f);
					direction = glm::normalize(direction);
					data.spotDirection = glm::vec4(direction, glm::cos(mAngle));
				} break;
			}

			context.lightData.lightBuffer[context.lightData.lightCount] = data;
			context.lightData.lightCount++;
		}
	}
}

void LightComponent::OnGUI()
{
	static LightType selectedType = LightType::POINT;
	const char* typeNames[] = { "Directional", "Point", "Spot"};

	ImGui::Combo("Select Type", reinterpret_cast<int *>(&selectedType), typeNames, IM_ARRAYSIZE(typeNames));

	ImGui::ColorEdit3("Color", reinterpret_cast<float *>(&mColor));

	switch (selectedType)
	{
		case LightType::POINT:
		{
		} break;
		case LightType::SPOT:
		{
			ImGui::SliderAngle("Angle", &mAngle, 0.0f, 90.0f);
		} break;
	}

	mType = selectedType;
}

ColliderComponent::ColliderComponent(Entity &parent): Component(parent)
{
	mType = ColliderType::INVALID;
}

SphereColliderComponent::SphereColliderComponent(Entity &parent) : ColliderComponent(parent)
{
	mComponentType = ComponentType::SPHERE_COLLIDER;
	mCollider.type = ColliderType::SPHERE;
	mCollider.radius = 1.0f;
}

void SphereColliderComponent::Update()
{
	auto transform = static_cast<TransformComponent *>(parent.GetComponent(ComponentType::TRANSFORM));
	if (transform != nullptr)
	{
		mCollider.position = transform->mPosition;
	}
}

void SphereColliderComponent::Render(RenderContext &context, ModelRenderData &renderData)
{
}

void SphereColliderComponent::OnGUI()
{
	ImGui::InputFloat("Radius", &mCollider.radius);
}

Collider* SphereColliderComponent::GetCollider()
{
	auto transform = static_cast<TransformComponent *>(parent.GetComponent(ComponentType::TRANSFORM));
	if (transform == nullptr)
	{
		mCollider.type = ColliderType::INVALID;
	}

	return &mCollider;
}

BoxColliderComponent::BoxColliderComponent(Entity &parent) : ColliderComponent(parent)
{
	mComponentType = ComponentType::BOX_COLLIDER;
	mCollider.type = ColliderType::BOX;
	mCollider.extents = glm::vec3(1.0f, 1.0f, 1.0f);
}

void BoxColliderComponent::Update()
{
	auto transform = static_cast<TransformComponent *>(parent.GetComponent(ComponentType::TRANSFORM));
	if (transform != nullptr)
	{
		mCollider.position = transform->mPosition;
	}
}

void BoxColliderComponent::Render(RenderContext &context, ModelRenderData &renderData)
{
}

void BoxColliderComponent::OnGUI()
{
	ImGui::InputFloat3("Extents", reinterpret_cast<float *>(&mCollider.extents));
}

Collider* BoxColliderComponent::GetCollider()
{
	auto transform = static_cast<TransformComponent *>(parent.GetComponent(ComponentType::TRANSFORM));
	if (transform == nullptr)
	{
		mCollider.type = ColliderType::INVALID;
	}

	return &mCollider;
}
