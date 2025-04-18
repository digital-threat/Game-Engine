#include <vk_immediate.h>
#include <vk_helpers.h>

VkCommandBuffer BeginImmediate(VkDevice device, ImmediateData& data)
{
	vkResetFences(device, 1, &data.fence);
	vkResetCommandBuffer(data.cmd, 0);

	VkCommandBufferBeginInfo cmdBeginInfo = CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	vkBeginCommandBuffer(data.cmd, &cmdBeginInfo);

	return data.cmd;
}

void EndImmediate(VkDevice device, VkQueue queue, ImmediateData& data)
{
	vkEndCommandBuffer(data.cmd);

	VkCommandBufferSubmitInfo cmdInfo = CommandBufferSubmitInfo(data.cmd);
	VkSubmitInfo2 submit = SubmitInfo(&cmdInfo, nullptr, nullptr);

	vkQueueSubmit2(queue, 1, &submit, data.fence);

	vkWaitForFences(device, 1, &data.fence, true, UINT64_MAX);
}

void ImmediateSubmit(VkDevice device, VkQueue queue, const ImmediateData& data, std::function<void(VkCommandBuffer cmd)> func)
{
	vkResetFences(device, 1, &data.fence);
	vkResetCommandBuffer(data.cmd, 0);

	VkCommandBufferBeginInfo cmdBeginInfo = CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	vkBeginCommandBuffer(data.cmd, &cmdBeginInfo);

	func(data.cmd);

	vkEndCommandBuffer(data.cmd);

	VkCommandBufferSubmitInfo cmdInfo = CommandBufferSubmitInfo(data.cmd);
	VkSubmitInfo2 submit = SubmitInfo(&cmdInfo, nullptr, nullptr);

	vkQueueSubmit2(queue, 1, &submit, data.fence);

	vkWaitForFences(device, 1, &data.fence, true, UINT64_MAX);
}
