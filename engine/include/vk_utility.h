#pragma once

#include <functional>
#include <vulkan/vulkan_core.h>

struct ImmediateData
{
	VkFence fence = VK_NULL_HANDLE;
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	VkCommandPool cmdPool = VK_NULL_HANDLE;
};

void ImmediateSubmit(VkDevice device, VkQueue queue, const ImmediateData& data, std::function<void(VkCommandBuffer cmd)> func);
