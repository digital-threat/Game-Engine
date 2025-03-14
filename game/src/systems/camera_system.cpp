#include <systems/camera_system.h>
#include <imgui.h>
#include <renderer_vk_types.h>
#include <components/camera.h>
#include <ecs/component_manager.h>
#include <ecs/entity_manager.h>
#include <ecs/typedefs.h>

void CameraSystem::Update(EntityManager& entityManager, ComponentManager& componentManager, SceneRenderData &context)
{
	Archetype archetype;
	archetype.set(componentManager.GetComponentType<Camera>());

	auto func = [&](Entity entity)
	{
		Camera& camera = componentManager.GetComponent<Camera>(entity);

		context.cameraPos = camera.position;
		context.cameraLookAt = camera.lookAt;
		context.cameraFOV = camera.fov;
	};

	entityManager.Each(archetype, func);
}

void CameraSystem::OnGUI(EntityManager &entityManager, ComponentManager &componentManager)
{
	Archetype archetype;
	archetype.set(componentManager.GetComponentType<Camera>());

	auto func = [&](Entity entity)
	{
		Camera& camera = componentManager.GetComponent<Camera>(entity);

		if (ImGui::Begin("Camera"))
		{
			ImGui::InputFloat3("Position:", reinterpret_cast<float *>(&camera.position));
			ImGui::InputFloat3("Look At:", reinterpret_cast<float *>(&camera.lookAt));
			ImGui::InputFloat("FOV", &camera.fov);
		}
		ImGui::End();
	};

	entityManager.Each(archetype, func);
}
