#include <input.h>

namespace Input
{
	namespace
	{
		GLFWwindow* _window;
		glm::vec2 _mousePosition;
		glm::vec2 _prevMousePosition;
		f32 _mouseScrollFactor = 1.0f;

		void MouseCallback(GLFWwindow* window, double x, double y)
		{
			_prevMousePosition = _mousePosition;
			_mousePosition.x = x;
			_mousePosition.y = y;
		}

		void ScrollCallback(GLFWwindow* window, double x, double y)
		{
			_mouseScrollFactor += y;
			if (_mouseScrollFactor < 0.0f)
			{
				_mouseScrollFactor = 0.0f;
			}
		}
	}

	void Initialize(GLFWwindow* window)
	{
		_window = window;

		int width, height = 0;

		glfwGetWindowSize(_window, &width, &height);

		_mousePosition.x = width / 2;
		_mousePosition.y = height / 2;

		glfwSetCursorPosCallback(_window, MouseCallback);
		glfwSetScrollCallback(_window, ScrollCallback);
	}

	bool GetKeyDown(u32 key)
	{
		return glfwGetKey(_window, key) == GLFW_PRESS;
	}

	bool GetButtonDown(u32 button)
	{
		assert(button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT);

		return glfwGetMouseButton(_window, button) == GLFW_PRESS;
	}

	glm::vec2 GetMousePosition()
	{
		return _mousePosition;
	}

	glm::vec2 GetMouseDelta()
	{
		return _prevMousePosition - _mousePosition;
	}

	f32 GetMouseScrollFactor()
	{
		return _mouseScrollFactor;
	}

	void HideCursor()
	{
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void ShowCursor()
	{
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}
