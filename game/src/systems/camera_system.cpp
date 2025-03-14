#include <systems/camera_system.h>
#include <imgui.h>
#include <input.h>
#include <renderer_vk_types.h>
#include <components/camera.h>
#include <ecs/component_manager.h>
#include <ecs/entity_manager.h>
#include <ecs/typedefs.h>

#include "glm/ext/scalar_constants.hpp"

void CameraSystem::Update(EntityManager& entityManager, ComponentManager& componentManager, SceneRenderData &context, f32 deltaTime)
{
	Archetype archetype;
	archetype.set(componentManager.GetComponentType<Camera>());

	auto func = [&](Entity entity)
	{
		Camera& camera = componentManager.GetComponent<Camera>(entity);

		glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);

		if (Input::GetKeyDown(GLFW_KEY_W)) velocity.z += 1;
		if (Input::GetKeyDown(GLFW_KEY_A)) velocity.x += -1;
		if (Input::GetKeyDown(GLFW_KEY_S)) velocity.z += -1;
		if (Input::GetKeyDown(GLFW_KEY_D)) velocity.x += 1;
		if (Input::GetKeyDown(GLFW_KEY_Q)) velocity.y += -1;
		if (Input::GetKeyDown(GLFW_KEY_E)) velocity.y += 1;

		if (glm::length(velocity) > 0.01f)
		{
			velocity = glm::normalize(velocity);
			camera.position += velocity * deltaTime;
		}

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
