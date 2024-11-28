#pragma once

#include <vulkan/vulkan_core.h>

namespace Renderer
{
	void TransitionImageLayout(VkCommandBuffer pCmd, VkImage pImage, VkImageLayout pOldLayout, VkImageLayout pNewLayout);

}
