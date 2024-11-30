#include "renderer_vk_pipelines.h"
#include "renderer_vk_structures.h"
#include "utility.h"
#include <fstream>
#include <vector>

namespace Renderer
{
	void LoadShaderModule(const char *pFilePath, const VkDevice pDevice, VkShaderModule *outShaderModule)
	{
		std::vector<u32> buffer;
		ReadFile(pFilePath, buffer);

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.codeSize = buffer.size() * sizeof(u32);
		createInfo.pCode = buffer.data();

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(pDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module.");
		}

		*outShaderModule = shaderModule;
	}
}
