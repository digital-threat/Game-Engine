#pragma once

#include <types.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

namespace Input
{
	void Initialize(GLFWwindow* window);
	bool GetKeyDown(u32 key);
	bool GetButtonDown(u32 button);
	glm::vec2 GetMousePosition();
	glm::vec2 GetMouseDelta();
	f32 GetMouseScrollFactor();
	void HideCursor();
	void ShowCursor();
}
