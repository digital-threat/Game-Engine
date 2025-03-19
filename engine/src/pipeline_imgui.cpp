#include <engine.h>
#include <vk_helpers.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>


void Engine::RenderImgui(VkCommandBuffer pCmd, VkImageView pTargetImageView)
{
	VkRenderingAttachmentInfo colorAttachment = ColorAttachmentInfo(pTargetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingInfo renderInfo = RenderingInfo(mVkbSwapchain.extent, &colorAttachment, nullptr);

	vkCmdBeginRendering(pCmd, &renderInfo);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), pCmd);

	vkCmdEndRendering(pCmd);
}
