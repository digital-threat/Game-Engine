#pragma once

#include <vulkan/vulkan_core.h>

namespace Renderer
{
	void LoadShaderModule(const char* pFilePath, VkDevice pDevice, VkShaderModule* outShaderModule);
}

