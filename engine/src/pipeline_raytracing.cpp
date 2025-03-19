#include <engine.h>

void Engine::InitializeRaytracingPipeline()
{
	VkPhysicalDeviceProperties2 properties2{};
	properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	properties2.pNext = &mRtProperties;
	vkGetPhysicalDeviceProperties2(mPhysicalDevice, &properties2);
}

void Engine::RenderRaytracing(VkCommandBuffer pCmd)
{
}