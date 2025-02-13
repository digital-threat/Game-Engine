#include "entity.h"

#include "components.h"
#include <imgui.h>

Entity::Entity(i32 id) : id(id)
{
}

void Entity::Update()
{
	for (Component *component: mComponents)
	{
		component->Update();
	}
}

void Entity::Render(RenderContext &renderContext)
{
	ModelRenderData renderData{};
	renderData.name = mName;

	for (Component *component: mComponents)
	{
		component->Render(renderContext, renderData);
	}

	if (renderData.indexCount == 0 || renderData.materialSet == VK_NULL_HANDLE)
	{
		return;
	}

	renderContext.modelData.push_back(renderData);
}

void Entity::OnGUI()
{
	static char nameBuffer[64]{};
	assert(mName.size() < 64);
	mName.copy(nameBuffer, mName.size());
	nameBuffer[mName.size()] = '\0';

	if (ImGui::InputText("Name", nameBuffer, IM_ARRAYSIZE(nameBuffer)))
	{
		mName = std::string(nameBuffer);
	}

	for (Component *component: mComponents)
	{
		component->OnGUI();
	}

	static ComponentType selectedType = ComponentType::TRANSFORM;
	const char* typeNames[] = { "Transform", "Mesh", "Light", "Sphere Collider", "Box Collider" };

	if (ImGui::Combo("Add Component", reinterpret_cast<int *>(&selectedType), typeNames, IM_ARRAYSIZE(typeNames)))
	{
		switch (selectedType)
		{
			case ComponentType::TRANSFORM:
			{
				TransformComponent* component = new TransformComponent(*this);
				AddComponent(component);
			} break;
			case ComponentType::MESH:
			{
				MeshComponent* component = new MeshComponent(*this);
				AddComponent(component);
			} break;
			case ComponentType::LIGHT:
			{
				LightComponent* component = new LightComponent(*this);
				AddComponent(component);
			} break;
			case ComponentType::SPHERE_COLLIDER:
			{
				SphereColliderComponent* component = new SphereColliderComponent(*this);
				AddComponent(component);
			} break;
			case ComponentType::BOX_COLLIDER:
			{
				BoxColliderComponent* component = new BoxColliderComponent(*this);
				AddComponent(component);
			} break;
		}
	}

	static int index = 0;
	ImGui::SliderInt("Index", &index,0, mComponents.size() - 1);
	ImGui::SameLine();
	if (ImGui::Button("Remove Component"))
	{
		RemoveComponent(index);
	}
}

void Entity::AddComponent(Component *component)
{
	if (GetComponent(component->mComponentType) == nullptr)
	{
		mComponents.push_back(component);
	}
}

void Entity::RemoveComponent(u32 index)
{
	if (index < mComponents.size() && index >= 0)
	mComponents.erase(mComponents.begin() + index);
}

Component* Entity::GetComponent(ComponentType type)
{
	for (Component *component : mComponents)
	{
		if (component->mComponentType == type)
		{
			return component;
		}
	}

	return nullptr;
}
