#include <sandbox.h>

#include <engine.h>
#include <application.h>

int main()
{
	Application* application = new MySandbox();
	Engine* engine = new Engine(application);

	try
	{
		engine->Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
