#include <engine.h>
#include <vk_helpers.h>
#include <vk_pipelines.h>
#include <glm/gtc/quaternion.hpp>

void Engine::InitShadowmapPipeline()
{
	VkShaderModule vertexShader;
	LoadShaderModule("shaders/shadowmap_vert.spv", mDevice, &vertexShader);

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(ShadowmapPushConstants);
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = PipelineLayoutCreateInfo();
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	VK_CHECK(vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mShadowmapPipelineLayout));

	PipelineBuilder builder;
	builder.mPipelineLayout = mShadowmapPipelineLayout;
	builder.SetShaders(vertexShader, VK_NULL_HANDLE);
	builder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	builder.SetPolygonMode(VK_POLYGON_MODE_FILL);
	builder.SetCullMode(VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_CLOCKWISE);
	builder.SetMultisamplingNone();
	builder.DisableBlending();
	builder.EnableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
	builder.SetDepthFormat(mShadowmapTarget.format);
	mShadowmapPipeline = builder.Build(mDevice);

	vkDestroyShaderModule(mDevice, vertexShader, nullptr);
}

void Engine::RenderShadowmap(VkCommandBuffer cmd)
{
	VkRenderingAttachmentInfo depthAttachment = DepthAttachmentInfo(mShadowmapTarget.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	VkRenderingInfo renderInfo = RenderingInfo(VkExtent2D(mShadowmapTarget.extent.width, mShadowmapTarget.extent.height), VK_NULL_HANDLE, &depthAttachment);

	vkCmdBeginRendering(cmd, &renderInfo);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mShadowmapPipeline);

	VkViewport viewport{};
	viewport.x = 0;
	viewport.y = static_cast<float>(mShadowmapTarget.extent.height);
	viewport.width = static_cast<float>(mShadowmapTarget.extent.width);
	viewport.height = -static_cast<float>(mShadowmapTarget.extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(cmd, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = mShadowmapTarget.extent.width;
	scissor.extent.height = mShadowmapTarget.extent.height;

	vkCmdSetScissor(cmd, 0, 1, &scissor);

	for (auto& model : mApplication->mRenderContext.instances)
	{
		ShadowmapPushConstants pushConstants;

		glm::vec3 lightPos = mApplication->mRenderContext.scene.mainLightPos;
		glm::mat4 matrixP = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 100.0f, 0.1f);
		glm::mat4 matrixV = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));

		pushConstants.depthMVP = matrixP * matrixV * model.transform;
		pushConstants.vertexBuffer = model.vertexBufferAddress;

		vkCmdPushConstants(cmd, mShadowmapPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ShadowmapPushConstants), &pushConstants);
		vkCmdBindIndexBuffer(cmd, model.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(cmd, model.indexCount, 1, 0, 0, 0);
	}

	vkCmdEndRendering(cmd);
}
