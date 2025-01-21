#include "entity.h"

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
		component->Render(renderData);
	}

	if (renderData.indexCount == 0 || renderData.materialSet == nullptr)
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

	if (ImGui::InputText("Name: ", nameBuffer, IM_ARRAYSIZE(nameBuffer)))
	{
		mName = std::string(nameBuffer);
	}

	for (Component *component: mComponents)
	{
		component->OnGUI();
	}
}

void Entity::AddComponent(Component *component)
{
	mComponents.push_back(component);
}
