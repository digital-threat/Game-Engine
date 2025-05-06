#include <components/camera.h>
#include <ecs/component_manager.h>
#include <ecs/entity_manager.h>
#include <ecs/typedefs.h>
#include <imgui.h>
#include <input.h>
#include <iostream>
#include <render_context.h>
#include <systems/camera_system.h>

void CameraSystem::Update(EntityManager& entityManager, ComponentManager& componentManager, CameraRenderData& context, f32 deltaTime)
{
	if (Input::GetButtonDown(GLFW_MOUSE_BUTTON_2))
	{
		Input::HideCursor();
	}
	else
	{
		Input::ShowCursor();
		return;
	}

	Archetype archetype;
	archetype.set(componentManager.GetComponentType<Camera>());

	auto func = [&](Entity entity)
	{
		Camera& camera = componentManager.GetComponent<Camera>(entity);

		f32 scrollFactor = Input::GetMouseScrollFactor();

		glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
		if (Input::GetKeyDown(GLFW_KEY_W))
			direction.z += 1;
		if (Input::GetKeyDown(GLFW_KEY_A))
			direction.x += -1;
		if (Input::GetKeyDown(GLFW_KEY_S))
			direction.z += -1;
		if (Input::GetKeyDown(GLFW_KEY_D))
			direction.x += 1;
		if (Input::GetKeyDown(GLFW_KEY_Q))
			direction.y += -1;
		if (Input::GetKeyDown(GLFW_KEY_E))
			direction.y += 1;

		glm::vec2 mousePos = Input::GetMousePosition();
		glm::vec2 mouseDelta = camera.prevMousePos - mousePos;
		camera.prevMousePos = mousePos;

		camera.yaw += mouseDelta.x * camera.sensitivity;
		camera.pitch += mouseDelta.y * camera.sensitivity;
		camera.pitch = glm::clamp(camera.pitch, -89.0f, 89.0f);

		glm::vec3 forward, right, up;
		forward.x = cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
		forward.y = sin(glm::radians(camera.pitch));
		forward.z = sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
		forward = glm::normalize(forward);

		right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), forward));
		up = glm::normalize(glm::cross(forward, right));

		if (glm::length(direction) > 0.0f)
		{
			direction = glm::normalize(direction);
			direction = direction.x * right + direction.y * up + direction.z * forward;
			camera.position += direction * camera.speed * scrollFactor * deltaTime;
		}

		context.pos = camera.position;
		context.forward = forward;
		context.up = up;
		context.fov = camera.fov;
	};

	entityManager.Each(archetype, func);
}

// NOTE(Sergei): Assumes there's only 1 camera.
void CameraSystem::OnGUI(EntityManager& entityManager, ComponentManager& componentManager)
{
	if (ImGui::CollapsingHeader("Camera"))
	{
		Archetype archetype;
		archetype.set(componentManager.GetComponentType<Camera>());

		auto func = [&](Entity entity)
		{
			Camera& camera = componentManager.GetComponent<Camera>(entity);


			ImGui::InputFloat3("Position", reinterpret_cast<float*>(&camera.position));
			ImGui::InputFloat("Speed", &camera.speed);
			ImGui::InputFloat("Sensitivity", &camera.sensitivity);
			ImGui::InputFloat("FOV", &camera.fov);
		};

		entityManager.Each(archetype, func);
	}
}
