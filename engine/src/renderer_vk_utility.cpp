#include "renderer_vk_utility.h"

#include <renderer_vk_structures.h>

namespace Renderer
{
	void ImmediateSubmit(const VkDevice& pDevice, const VkQueue& pQueue, const ImmediateData& pData, std::function<void(VkCommandBuffer cmd)> &&pFunction)
	{
		vkResetFences(pDevice, 1, &pData.fence);
		vkResetCommandBuffer(pData.cmd, 0);

		VkCommandBufferBeginInfo cmdBeginInfo = CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		vkBeginCommandBuffer(pData.cmd, &cmdBeginInfo);

		pFunction(pData.cmd);

		vkEndCommandBuffer(pData.cmd);

		VkCommandBufferSubmitInfo cmdInfo = CommandBufferSubmitInfo(pData.cmd);
		VkSubmitInfo2 submit = SubmitInfo(&cmdInfo, nullptr, nullptr);

		vkQueueSubmit2(pQueue, 1, &submit, pData.fence);

		vkWaitForFences(pDevice, 1, &pData.fence, true, UINT64_MAX);
	}
}
