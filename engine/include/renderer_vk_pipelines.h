#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

namespace Renderer
{
	void LoadShaderModule(const char* pFilePath, VkDevice pDevice, VkShaderModule* outShaderModule);

	class PipelineBuilder
	{
	public:
		std::vector<VkPipelineShaderStageCreateInfo> mShaderStages;

		VkPipelineInputAssemblyStateCreateInfo mInputAssembly;
		VkPipelineRasterizationStateCreateInfo mRasterizer;
		VkPipelineColorBlendAttachmentState mColorBlendAttachment;
		VkPipelineMultisampleStateCreateInfo mMultisampling;
		VkPipelineLayout mPipelineLayout;
		VkPipelineDepthStencilStateCreateInfo mDepthStencil;
		VkPipelineRenderingCreateInfo mRenderInfo;
		VkFormat mColorAttachmentFormat;

		PipelineBuilder()
		{
			Clear();
		}

		void Clear();

		void SetShaders(VkShaderModule pVertexShader, VkShaderModule pFragmentShader);
		void SetInputTopology(VkPrimitiveTopology pTopology);
		void SetPolygonMode(VkPolygonMode pMode);
		void SetCullMode(VkCullModeFlags pCullMode, VkFrontFace pFrontFace);
		void SetMultisamplingNone();
		void DisableBlending();
		void EnableBlendingAdditive();
		void EnableBlendingAlpha();
		void SetColorAttachmentFormat(VkFormat pFormat);
		void SetDepthFormat(VkFormat pFormat);
		void DisableDepthTest();
		void EnableDepthTest(bool pDepthWriteEnable, VkCompareOp pOp);

		VkPipeline Build(VkDevice pDevice);
	};
}

