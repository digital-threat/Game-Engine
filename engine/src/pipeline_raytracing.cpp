#include <engine.h>

void Engine::InitRaytracing()
{
	VkPhysicalDeviceProperties2 properties2{};
	properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	properties2.pNext = &mRtProperties;
	vkGetPhysicalDeviceProperties2(mPhysicalDevice, &properties2);
}

void Engine::InitRaytracingDescriptorLayout()
{
	DescriptorLayoutBuilder builder;
	builder.AddBinding(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);
	builder.AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	mRaytracingDescriptorLayout = builder.Build(mDevice, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
}

void Engine::InitRaytracingPipeline()
{

}

void Engine::RenderRaytracing(VkCommandBuffer cmd, FrameData& currentFrame)
{
	VkDescriptorSet raytracingSet = currentFrame.descriptorAllocator.Allocate(mDevice, mSceneDescriptorLayout);

	DescriptorWriter writer;
	writer.WriteTlas(0, mApplication->mRenderContext.raytracing.tlas);
	// writer.
	//
	// VkWriteDescriptorSetAccelerationStructureKHR descASInfo{};
	// descASInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	// descASInfo.accelerationStructureCount = 1;
	// descASInfo.pAccelerationStructures = &mApplication->mRenderContext.raytracing.tlas;
	// VkDescriptorImageInfo imageInfo{{}, m_offscreenColor.descriptor.imageView, VK_IMAGE_LAYOUT_GENERAL};
	//
	// std::vector<VkWriteDescriptorSet> writes;
	// writes.emplace_back(m_rtDescSetLayoutBind.makeWrite(m_rtDescSet, RtxBindings::eTlas, &descASInfo));
	// writes.emplace_back(m_rtDescSetLayoutBind.makeWrite(m_rtDescSet, RtxBindings::eOutImage, &imageInfo));
	// vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

	writer.WriteBuffer(0, currentFrame.sceneDataBuffer.buffer, sizeof(SceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	writer.WriteImage(1, mShadowmapTarget.imageView, TextureManager::Get().GetSampler("NEAREST_MIPMAP_LINEAR"), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	writer.UpdateSet(mDevice, raytracingSet);

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mMeshPipelineLayout, 0, 1, &raytracingSet, 0, nullptr);
}