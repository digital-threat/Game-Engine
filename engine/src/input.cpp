#include <input.h>

namespace Input
{
	namespace
	{
		GLFWwindow* _window;
		glm::vec2 _mousePosition;

		void MouseCallback(GLFWwindow* window, double x, double y)
		{
			_mousePosition.x = x;
			_mousePosition.y = y;
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
}
