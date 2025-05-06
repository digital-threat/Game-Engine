#include <components/name.h>
#include <components/transform.h>
#include <ecs/component_manager.h>
#include <ecs/entity_manager.h>
#include <ecs/typedefs.h>
#include <imgui.h>
#include <systems/transform_gui_system.h>

void TransformGUISystem::Update(EntityManager& entityManager, ComponentManager& componentManager)
{
	if (ImGui::CollapsingHeader("Entities"))
	{
		Archetype archetype;
		archetype.set(componentManager.GetComponentType<Transform>());
		archetype.set(componentManager.GetComponentType<Name>());

		auto func = [&](Entity entity)
		{
			Transform& transform = componentManager.GetComponent<Transform>(entity);
			Name& name = componentManager.GetComponent<Name>(entity);

			if (ImGui::TreeNode((name.name + "##" + std::to_string(entity)).c_str()))
			{
				ImGui::DragFloat3("Position", reinterpret_cast<float*>(&transform.position), 0.1f);
				ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&transform.rotation), 0.1f);
				ImGui::DragFloat("Scale", &transform.scale, 0.1f);
				ImGui::TreePop();
			}
		};

		entityManager.Each(archetype, func);
	}
}
