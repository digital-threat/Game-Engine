#pragma once

#include <functional>
#include <renderer_vk_types.h>
#include <vulkan/vulkan_core.h>

void ImmediateSubmit(const VkDevice& pDevice, const VkQueue& pQueue, const ImmediateData& pData, std::function<void(VkCommandBuffer cmd)>&& pFunction);
