#include <forward_renderer.h>
#include <engine.h>
#include <application.h>
#include <iostream>

int main()
{
	Engine* engine = new Engine();
	Application* application = new ForwardRenderer(*engine);

	try
	{
		engine->Run(application);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
