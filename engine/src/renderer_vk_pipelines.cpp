#include "renderer_vk_pipelines.h"

#include <array>

#include "renderer_vk_structures.h"
#include "utility.h"
#include <fstream>
#include <iostream>
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

	void PipelineBuilder::Clear()
	{
		mInputAssembly = {};
		mInputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mRasterizer = {};
		mRasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		mColorBlendAttachment = {};
		mMultisampling = {};
		mMultisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		mPipelineLayout = {};
		mDepthStencil = {};
		mDepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		mRenderInfo = {};
		mRenderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		mShaderStages.clear();
	}

	void PipelineBuilder::SetShaders(VkShaderModule pVertexShader, VkShaderModule pFragmentShader)
	{
		mShaderStages.clear();
		mShaderStages.push_back(PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, pVertexShader, "main"));
		mShaderStages.push_back(PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, pFragmentShader, "main"));
	}

	void PipelineBuilder::SetInputTopology(VkPrimitiveTopology pTopology)
	{
		mInputAssembly.topology = pTopology;
		mInputAssembly.primitiveRestartEnable = VK_FALSE;
	}

	void PipelineBuilder::SetPolygonMode(VkPolygonMode pMode)
	{
		mRasterizer.polygonMode = pMode;
		mRasterizer.lineWidth = 1.0f;
	}

	void PipelineBuilder::SetCullMode(VkCullModeFlags pCullMode, VkFrontFace pFrontFace)
	{
		mRasterizer.cullMode = pCullMode;
		mRasterizer.frontFace = pFrontFace;
	}

	void PipelineBuilder::SetMultisamplingNone()
	{
		mMultisampling.sampleShadingEnable = VK_FALSE;
		mMultisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		mMultisampling.minSampleShading = 1.0f;
		mMultisampling.pSampleMask = nullptr;
		mMultisampling.alphaToCoverageEnable = VK_FALSE;
		mMultisampling.alphaToOneEnable = VK_FALSE;
	}

	void PipelineBuilder::DisableBlending()
	{
		mColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		mColorBlendAttachment.blendEnable = VK_FALSE;
	}

	void PipelineBuilder::SetColorAttachmentFormat(VkFormat pFormat)
	{
		mColorAttachmentFormat = pFormat;
		mRenderInfo.colorAttachmentCount = 1;
		mRenderInfo.pColorAttachmentFormats = &mColorAttachmentFormat;
	}

	void PipelineBuilder::SetDepthFormat(VkFormat pFormat)
	{
		mRenderInfo.depthAttachmentFormat = pFormat;
	}

	void PipelineBuilder::DisableDepthTest()
	{
		mDepthStencil.depthTestEnable = VK_FALSE;
		mDepthStencil.depthWriteEnable = VK_FALSE;
		mDepthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
		mDepthStencil.depthBoundsTestEnable = VK_FALSE;
		mDepthStencil.stencilTestEnable = VK_FALSE;
		mDepthStencil.front = {};
		mDepthStencil.back = {};
		mDepthStencil.minDepthBounds = 0.0f;
		mDepthStencil.maxDepthBounds = 1.0f;
	}

	void PipelineBuilder::EnableDepthTest(bool pDepthWriteEnable, VkCompareOp pOp)
	{
		mDepthStencil.depthTestEnable = VK_TRUE;
		mDepthStencil.depthWriteEnable = pDepthWriteEnable;
		mDepthStencil.depthCompareOp = pOp;
		mDepthStencil.depthBoundsTestEnable = VK_FALSE;
		mDepthStencil.stencilTestEnable = VK_FALSE;
		mDepthStencil.front = {};
		mDepthStencil.back = {};
		mDepthStencil.minDepthBounds = 0.0f;
		mDepthStencil.maxDepthBounds = 1.0f;
	}

	VkPipeline PipelineBuilder::Build(VkDevice pDevice)
	{
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.pNext = nullptr;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.pNext = nullptr;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &mColorBlendAttachment;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		std::array<VkDynamicState, 2> state = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicInfo{};
		dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicInfo.pDynamicStates = state.data();
		dynamicInfo.dynamicStateCount = state.size();

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = &mRenderInfo;
		pipelineInfo.stageCount = static_cast<u32>(mShaderStages.size());
		pipelineInfo.pStages = mShaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &mInputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &mRasterizer;
		pipelineInfo.pMultisampleState = &mMultisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDepthStencilState = &mDepthStencil;
		pipelineInfo.layout = mPipelineLayout;
		pipelineInfo.pDynamicState = &dynamicInfo;

		VkPipeline pipeline;
		if (vkCreateGraphicsPipelines(pDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphics pipeline.");
		}

		return pipeline;
	}
}
