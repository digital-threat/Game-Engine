#include <engine.h>
#include <iostream>

void Engine::InitRaytracing()
{
	VkPhysicalDeviceProperties2 properties2{};
	properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	properties2.pNext = &mRtProperties;
	vkGetPhysicalDeviceProperties2(mPhysicalDevice, &properties2);
}

void Engine::InitRaytracingPipeline()
{

}

void Engine::RenderRaytracing(VkCommandBuffer cmd)
{
}