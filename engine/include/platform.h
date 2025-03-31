#pragma once

//#ifdef PLATFORM_WINDOWS
//
//extern Application* CreateApplication();
//
//int main(int argc, char** argv)
//{
//    auto application = CreateApplication();
//  	application->Run();
//}
//
//#endif

#ifdef GRAPHICS_API_VULKAN
	#include <vulkan/buffer.h>
	#include <vulkan/command_buffer.h>
	#include <vulkan/texture.h>
#elif GRAPHICS_API_D3D12
	#error D3D12 is not supported!
#endif
