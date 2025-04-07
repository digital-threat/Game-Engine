#include <engine.h>
#include <vk_helpers.h>
#include <vk_pipelines.h>

void Engine::InitBackgroundDescriptorLayout()
{
	DescriptorLayoutBuilder builder;
	builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT);
	mBackground.descriptorLayout = builder.Build(mDevice);
}

void Engine::UpdateBackgroundDescriptorSet()
{
	mBackground.descriptorSet = mGlobalDescriptorAllocator.Allocate(mDevice, mBackground.descriptorLayout);

	DescriptorWriter writer;
	writer.WriteImage(0, mColorTarget.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	writer.UpdateSet(mDevice, mBackground.descriptorSet);
}

void Engine::InitBackgroundPipeline()
{
	VkPushConstantRange pushConstant{};
	pushConstant.offset = 0;
	pushConstant.size = sizeof(ComputePushConstants);
	pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	VkPipelineLayoutCreateInfo computeLayout{};
	computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	computeLayout.pNext = nullptr;
	computeLayout.pSetLayouts = &mBackground.descriptorLayout;
	computeLayout.setLayoutCount = 1;
	computeLayout.pPushConstantRanges = &pushConstant;
	computeLayout.pushConstantRangeCount = 1;

	if (vkCreatePipelineLayout(mDevice, &computeLayout, nullptr, &mBackground.pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout.");
	}

	VkShaderModule gradientShader;
	LoadShaderModule("shaders/gradient_color.spv", mDevice, &gradientShader);

	VkPipelineShaderStageCreateInfo stageInfo = PipelineShaderStageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT, gradientShader, "main");

	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.layout = mBackground.pipelineLayout;
	computePipelineCreateInfo.stage = stageInfo;

	ComputeEffect gradient{};
	gradient.layout = mBackground.pipelineLayout;
	gradient.name = "gradient";
	gradient.data.data1 = glm::vec4(0, 0, 0, 1);
	gradient.data.data2 = glm::vec4(0, 0, 0, 1);

	if (vkCreateComputePipelines(mDevice, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &gradient.pipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create compute pipeline.");
	}

	mBackground.effects.push_back(gradient);

	vkDestroyShaderModule(mDevice, gradientShader, nullptr);
}

void Engine::RenderBackground(VkCommandBuffer cmd)
{
	ComputeEffect& effect = mBackground.effects[mBackground.currentEffect];

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, effect.pipeline);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, mBackground.pipelineLayout, 0, 1, &mBackground.descriptorSet, 0, nullptr);

	vkCmdPushConstants(cmd, mBackground.pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &effect.data);

	vkCmdDispatch(cmd, std::ceil(mRenderExtent.width / 16.0), std::ceil(mRenderExtent.height / 16.0), 1);
}

void Engine::Render(FrameData& currentFrame)
{
	//TransitionImageLayout(cmd, mColorTarget.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	//RenderBackground(cmd);
}