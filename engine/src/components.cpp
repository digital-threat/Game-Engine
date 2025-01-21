#include "components.h"

#include <imgui.h>
#include <material_manager.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <filesystem>
#include <mesh_manager.h>

void TransformComponent::Update()
{
}

void TransformComponent::Render(ModelRenderData &renderData)
{
	glm::mat4 matrixM = glm::translate(glm::mat4(1.0f), mPosition);
	glm::quat rotation = glm::quat(glm::radians(mRotation));
	matrixM *= glm::toMat4(rotation);
	matrixM = glm::scale(matrixM, glm::vec3(mScale));

	renderData.transform = matrixM;
}

void TransformComponent::OnGUI()
{
	ImGui::InputFloat3("Position:", reinterpret_cast<float *>(&mPosition));
	ImGui::InputFloat3("Rotation:", reinterpret_cast<float *>(&mRotation));
	ImGui::InputFloat("Scale", &mScale);
}

void MeshComponent::Update()
{
	ProcessMessages();
}

void MeshComponent::Render(ModelRenderData &renderData)
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
